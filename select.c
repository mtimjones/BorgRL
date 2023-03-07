#include "headers.h"
#include "ces.h"
#include "stats.h"

typedef enum
{
   NO_OBJECT = 0,
   MAP_OBJECT = 1,
   INV_OBJECT = 2,
} window_object_t;

typedef struct selection_s
{
   bool selection;
   int  entity_selected;

} selection_t;

static selection_t selected_object = { false, -1 };

static void clear_selection( void )
{
   selected_object.selection = false;
   selected_object.entity_selected = -1;
}

bool get_inv_selection( int *entity )
{
   *entity = selected_object.entity_selected;
   return selected_object.selection;
}

static void heal_drone( )
{
   if ( selected_object.selection )
   {
      {
         if ( get_entity_health( selected_object.entity_selected ) < 
               get_entity_max_health( selected_object.entity_selected ) )
         {
            start_entity_healing( selected_object.entity_selected );
            add_message( "Started healing drone %d", selected_object.entity_selected );
         }
      }
   }
   else
   {
      add_message( "Left-click a docked drone to heal." );
   }

   return;
}

static void recycle_drone( )
{
   if ( selected_object.selection )
   {
      if ( selected_object.entity_selected != 0 )
      {
         if ( is_entity_docked( selected_object.selection ) )
         {
            int resources = 0;

            resources  = get_entity_health( selected_object.entity_selected );
            resources += get_entity_resources( selected_object.entity_selected );
            resources += get_entity_attack( selected_object.entity_selected );
            resources += get_entity_armor( selected_object.entity_selected );
            resources += get_entity_level( selected_object.entity_selected );

            add_message( "Drone recycled for %d resources.", resources );

            add_drones_recycled( 1 );
            add_drones_recycled_resources( resources );

            resources += get_entity_resources( PLAYER_ID );
            set_entity_resources( PLAYER_ID, resources );

            destroy_entity( selected_object.entity_selected );
            clear_selection( );
            return;
         }
      }
   }
   else
   {
      add_message( "Left-click a docked drone to recycle." );
      return;
   }

   add_message( "Could not perform drone recycle." );
}

float get_assimilate_probability( int attacker_entity, int attackee_entity )
{
   int attacker = get_entity_level( attacker_entity ) * get_entity_health( attacker_entity );
   int attackee = get_entity_level( attackee_entity ) * get_entity_health( attackee_entity );

   if ( attacker_entity == PLAYER_ID ) attacker += 2;

   float p = (float)attacker / (float)( attacker + attackee );

   return p;
}


static void assimilate_drone( )
{
   if ( selected_object.selection )
   {
      if ( is_entity_enemy( selected_object.entity_selected ) )
      {
         if ( entity_distance( PLAYER_ID, selected_object.entity_selected ) < 20 )
         {
            int resources_needed = ASSIMILATE_COST;
            if ( get_entity_resources( PLAYER_ID ) >= resources_needed )
            {
               // Deduct resources for the assimilate attempt
               set_entity_resources( PLAYER_ID, get_entity_resources( PLAYER_ID ) - resources_needed );

               // Probabilistic check of assimilation...
               float p = get_assimilate_probability( PLAYER_ID, selected_object.entity_selected );
               if ( getSRand( ) < p )
               {
                  assimilate( selected_object.entity_selected );
               }
               else
               {
                  add_message( "Assimilate attempt failed." );
               }

               clear_selection( );
               return;
            }
            else
            {
                add_message( "Insufficient resources to assimilate (%d).", resources_needed );
                return;
            }
         }
         else
         {
            add_message( "Drone is too far away to assimilate." );
            return;
         }
      }
   }

   add_message( "Left-click an enemy drone to assimilate." );
}

static void internal_dock_drone( int entity )
{
    set_target_dock( entity, TYPE_ENTITY_DOCK, PLAYER_ID );

    // Transfer resources, if any.
    if ( get_entity_resources( entity ) )
    {
       if ( is_entity_scavenger( entity ) ) add_resources_scavenged( get_entity_resources( entity ) );
       else add_resources_mined( get_entity_resources( entity ) );

       set_entity_resources( PLAYER_ID, ( get_entity_resources( PLAYER_ID ) + 
                                          get_entity_resources( entity ) ) );
    }

    return;
}

static void dock_drone( )
{
   if ( ( selected_object.selection ) && ( selected_object.entity_selected != 0 ) )
   {
      if ( !is_entity_docked( selected_object.entity_selected ) )
      {
         if ( is_entity_friendly( selected_object.entity_selected ) )
         {
             internal_dock_drone( selected_object.entity_selected );
         }
      }
      clear_selection( );
      return;
   }
   else
   {
      add_message( "Left-click a docked drone to dock." );
      return;
   }

   add_message( "Could not execute docking manuever." );
}

static void dock_all_drones( void )
{
    for ( int entity = 0 ; entity < MAX_ENTITIES ; entity++ )
    {
        if ( is_friendly_combat_drone( entity ) )
        {
             internal_dock_drone( entity );
        }
    }
}

static void direct_drone( int source_entity, int target_col, int target_row )
{
   int target_entity = get_entity_at( target_col, target_row );
   int col, row;

   if ( target_entity != NO_ENTITY )
   {
      if ( !is_target_compatible( selected_object.entity_selected, target_entity ) )
      {
         clear_selection( );
         return;
      }
   }

   // Is the drone docked in the Borg?
   if ( is_entity_docked( selected_object.entity_selected ) )
   {
      col = get_entity_col( source_entity );
      row = get_entity_row( source_entity );

      // Find an empty map cell around the source entity.
      if ( find_empty_cell_around_point( &col, &row ) )
      {
         set_entity_state_str( selected_object.entity_selected, "Undocked  " );

         // Deploy the drone
         set_entity_col( selected_object.entity_selected, col );
         set_entity_row( selected_object.entity_selected, row );
         reset_entity_docked( selected_object.entity_selected );
         set_cell_entity( col, row, selected_object.entity_selected );

         if ( target_entity == NO_ENTITY )
         {
            // Targeting an empty space on the map.
            set_target_loc( selected_object.entity_selected, TYPE_LOC_TARGET, target_col, target_row );
         }
         else
         {
            // Targeting an entity.
            if ( is_target_compatible( selected_object.entity_selected, target_entity ) )
            {
                set_entity_target( selected_object.entity_selected, target_entity );
                start_entity_behavior( selected_object.entity_selected );
            }
         }

         // Healing can only occur when docked.
         stop_entity_healing( selected_object.entity_selected );
      }

      clear_selection( );
   }
   else
   {
      // Drone is on the map.
      set_entity_state_str( selected_object.entity_selected, "Undocked  " );

      int target_entity = get_entity_at( target_col, target_row );
      if ( target_entity == NO_ENTITY )
      {
         // Targeting an empty space on the map.
         set_target_loc( selected_object.entity_selected, TYPE_LOC_TARGET, target_col, target_row );
      }
      else
      {
         // Targeting an entity.
         if ( target_entity == PLAYER_ID )
         {
            set_target_dock( selected_object.entity_selected, TYPE_ENTITY_DOCK, PLAYER_ID );
         }
         else
         {
            if ( is_target_compatible( selected_object.entity_selected, target_entity ) )
            {
               set_entity_target( selected_object.entity_selected, target_entity );
               start_entity_behavior( selected_object.entity_selected );
            }
         }
      }
      clear_selection( );
   }

  // Need to implement autonomous behavior (in behaviors.c)
     // For combat, attack target selected.
     // For bomb, go to target location and then explode (or entity).
     // For medic, seek out drones with non-max health, and deliver healing (cost resources).
     // For emp, seek out enemy and stay at a distance that allows them to be jammed.
}

static void map_information( void )
{
   // If an object was previously selected
   if ( selected_object.selection )
   {
      emit_map_context_info( selected_object.entity_selected );
   }
}

static void morph_drone( char type )
{
   // If a drone was previously selected
   if ( selected_object.selection )
   {
      // Is the drone docked in the Borg?
      if ( is_entity_docked( selected_object.entity_selected ) )
      {
         if ( is_entity_cdrone( selected_object.entity_selected ) )
         {
            if ( get_entity_resources( PLAYER_ID ) >= 7 ) 
            {
               set_morph_data( selected_object.entity_selected, MORPH_TIME, type );
               set_entity_resources( PLAYER_ID, ( get_entity_resources( PLAYER_ID ) - 7 ) );
               return;
            }
            else
            {
               add_message( "Drone morphing requires 7 resources." );
               return;
            }
         }
      }
   }

   add_message( "Left-click a docked combat drone to morph." );
}

void process_command( char cmd )
{
   bool cmd_processed = false;

   switch( cmd )
   {
      case 27: // ESC
         set_context( "" );
         cmd_processed = true;
         break;

      case '+':
         // TEST ONLY
         cmd_processed = true;
         set_entity_resources( PLAYER_ID, get_entity_resources( PLAYER_ID ) + 10 );
         break;

      case '?':
         emit_help( );
         cmd_processed = true;
         break;
      case 'l':
         emit_legend( );
         cmd_processed = true;
         break;

      case 'p':
         pause_toggle( );
         break;
      case ' ':
         turn_step( );
         break;

      case 'h':
         heal_drone( );
         cmd_processed = true;
         break;
      case 'd':
         dock_drone( );
         cmd_processed = true;
         break;
      case 'r':
         recycle_drone( );
         cmd_processed = true;
         break;
      case 'a':
         assimilate_drone( );
         cmd_processed = true;
         break;
      case 'D':
         dock_all_drones( );
         cmd_processed = true;
         break;

      case 's':
      case 'm':
      case 'j':
         morph_drone( cmd );
         cmd_processed = true;
         break;

      case 'i':
         map_information( );
         cmd_processed = true;
         break;

      case 'X':
         set_death_type( Surrendered );
         end_game( );
         break;
   }

   if ( cmd_processed )
   {
      clear_selection( );
   }

   return;
}

static void process_mouse_click( window_object_t wobject, int state, int col, int row )
{
   if ( wobject == MAP_OBJECT )
   {
      if ( state == BUTTON3_PRESSED )
      {
         if ( selected_object.selection == false )
         {
            // Set the Borg target.
            set_target_loc( PLAYER_ID, TYPE_LOC_TARGET, col, row );
         }
         else
         {
            // We have an entity selected, so this right-click is for it.
            direct_drone( PLAYER_ID, col, row );
         }
      }
      else if ( state == BUTTON1_PRESSED )
      {
         int entity = get_entity_at( col, row );
         if ( entity == NO_ENTITY )
         {
            clear_selection( );
         }
         else
         {
            selected_object.selection = true;
            selected_object.entity_selected = entity; 
         }

      }
   }
   else if ( wobject == INV_OBJECT )
   {
      if ( !is_entity_morphing( map_inv_index_to_entity( row ) ) )
      {
         selected_object.selection = true;
         selected_object.entity_selected = map_inv_index_to_entity( row ); // Entity passed here
      }
   }
}

void map_button_press( int col, int row, int state )
{
   int col_selected, row_selected;
   const int map_col_midpoint = ( ( MAPWIN_COL_SIZE-2 ) >> 1 );
   const int map_row_midpoint = ( ( MAPWIN_ROW_SIZE-2 ) >> 1 );

   col_selected = get_entity_col( PLAYER_ID ) - map_col_midpoint + col-1;
   row_selected = get_entity_row( PLAYER_ID ) - map_row_midpoint + row-1;

   if ( valid_map_location( col_selected, row_selected ) )
   {
      if ( state == BUTTON1_PRESSED )
      {
         //add_message( "Left-click select of map object at %d,%d", col_selected, row_selected );
         process_mouse_click( MAP_OBJECT, state, col_selected, row_selected );
      }
      else if ( state == BUTTON3_PRESSED )
      {
         //add_message( "Right-click select of map object at %d,%d", col_selected, row_selected );
         process_mouse_click( MAP_OBJECT, state, col_selected, row_selected );

      }
   }
}

void inv_button_press( int col, int row, int state )
{
   (void)col;

   if ( state == BUTTON1_PRESSED )
   {
      if ( row >= 2 )
      {
         int row_selected = row - 2;
         // add_message( "Left-click select of inv item %d = %d", row_selected, map_inv_index_to_entity( row_selected ) );
         process_mouse_click( INV_OBJECT, state, col, row_selected );
      }
   }
}

/*
   Esc, clear 1/2, clear context

   NO_COMMAND: 

    * Right-Click on map - move Borg to location.

    * Left-Click on inv item (entity), right click on map to direct (location or entity).

    * Left-Click on (friendly) map entity, right click on map location / entity to direct.

    * Left-Click on inv item (entity), press 'h' to heal.

    * Left-Click on inv item (entity), press 'r' to recycle.

    * Left-Click on (friendly) map entity, press 'd' to dock.

    * Left-Click on (enemy) map entity, press 'a' to assimilate.

*/

