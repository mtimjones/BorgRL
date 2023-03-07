#include "headers.h"

static int mouse_col, mouse_row;

void handle_user_input( void )
{
   int c;
   MEVENT event;

   // Check keyboard input.
   c = get_user_char( );

   if ( c == 0 )
   {
      // No character received.
      return;
   }
   else if ( c == KEY_MOUSE )
   {
      // Handle mouse event.
      if ( getmouse( &event ) == OK )
      {
         mouse_col = event.x; 
         mouse_row = event.y;

         // Check to see if the context needs to be updated (based upon mouse position).
         if ( event.x >= LOGWIN_COL_START && 
              event.x < (LOGWIN_COL_START+LOGWIN_COL_SIZE) && 
              event.y >= LOGWIN_ROW_START && 
              event.y < (LOGWIN_ROW_START+LOGWIN_ROW_SIZE) )
         {
             set_context( "Log window..." );
         }
         else if ( event.x >= MAPWIN_COL_START && 
                   event.x < (MAPWIN_COL_START+MAPWIN_COL_SIZE) && 
                   event.y >= MAPWIN_ROW_START && 
                   event.y < (MAPWIN_ROW_START+MAPWIN_ROW_SIZE) )
         {
             set_context( "Map window showing your surroundings and undocked drones." );
         }
         else if ( event.x >= INVWIN_COL_START && 
                   event.x < (INVWIN_COL_START+INVWIN_COL_SIZE) && 
                   event.y >= INVWIN_ROW_START && 
                   event.y < (INVWIN_ROW_START+INVWIN_ROW_SIZE) )
         {
             set_context( "Inventory window showing docked drones and their attributes." );
         }
         else if ( event.x >= STATSWIN_COL_START && 
                   event.x < (STATSWIN_COL_START+STATSWIN_COL_SIZE) && 
                   event.y >= STATSWIN_ROW_START && 
                   event.y < (STATSWIN_ROW_START+STATSWIN_ROW_SIZE) )
         {
             set_context( "Stats window showing various game resources and Borg upgrades." );
         }
         else if ( event.x >= ACTIONSWIN_COL_START && 
                   event.x < (ACTIONSWIN_COL_START+ACTIONSWIN_COL_SIZE) && 
                   event.y >= ACTIONSWIN_ROW_START &&
                   event.y < (ACTIONSWIN_ROW_START+ACTIONSWIN_ROW_SIZE) )
         {
             set_context( "Actions window..." );
         }
         else
         {
            // If the mouse position isn't in a contextual area, ignore it and clear context.
             clear_context( );
         }

         // Handle mouse clicks
         if ( event.bstate == BUTTON1_PRESSED )
         {
            // Check to see if the button was pressed in the map or inventory windows.

            if ( event.x >= MAPWIN_COL_START && 
                 event.x < (MAPWIN_COL_START+MAPWIN_COL_SIZE) && 
                 event.y >= MAPWIN_ROW_START && 
                 event.y < (MAPWIN_ROW_START+MAPWIN_ROW_SIZE) )
            {
               map_button_press( event.x, event.y, event.bstate );
            } 
            else if ( event.x >= INVWIN_COL_START && 
                      event.x < (INVWIN_COL_START+INVWIN_COL_SIZE) && 
                      event.y >= INVWIN_ROW_START && 
                      event.y < (INVWIN_ROW_START+INVWIN_ROW_SIZE) )
            {
               inv_button_press( event.x, event.y, event.bstate );
            }
         }
         else if ( event.bstate == BUTTON3_PRESSED )
         {
            // Check to see if the button was pressed in the map or inventory windows.

            if ( event.x >= MAPWIN_COL_START && 
                 event.x < (MAPWIN_COL_START+MAPWIN_COL_SIZE) && 
                 event.y >= MAPWIN_ROW_START && 
                 event.y < (MAPWIN_ROW_START+MAPWIN_ROW_SIZE) )
            {
               map_button_press( event.x, event.y, event.bstate );
            } 
            else if ( event.x >= INVWIN_COL_START && 
                      event.x < (INVWIN_COL_START+INVWIN_COL_SIZE) && 
                      event.y >= INVWIN_ROW_START && 
                      event.y < (INVWIN_ROW_START+INVWIN_ROW_SIZE) )
            {
               inv_button_press( event.x, event.y, event.bstate );
            }

          }
      }
   }
   else
   {
      process_command( c );
   }

   return;
}

int get_mouse_col( void )
{
   return mouse_col;
}

int get_mouse_row( void )
{
   return mouse_row;
}

