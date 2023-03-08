// ces.c
#include "headers.h"
#include "ces.h"
#include "behaviors.h"
#include "logapi.h"
#include "stats.h"
#include <math.h>

static World world;

int get_entity_at( int col, int row )
{
    int entity;

    for ( entity = 0 ; entity < MAX_ENTITIES ; entity++ )
    {
        if ( world.mask[ entity ] != COMPONENT_NONE )
        {
            if ( world.location[ entity ].col == col && world.location[ entity ].row == row )
            {
                return entity;
            }
        }
    }

    return NO_ENTITY;
}

int get_free_entity( void )
{
    int entity;

    for ( entity = 1 ; entity < MAX_ENTITIES ; entity++ )
    {
        if ( world.mask[ entity ] == COMPONENT_NONE ) break;
    }

    return entity;
}

void destroy_entity( int entity )
{
    world.mask[ entity ] = COMPONENT_NONE;

    world.callbacks[ entity ].on_attack = NULL;
    world.behavior[ entity ].behavior = NULL;

    return;
}

void find_empty_space( int col_sec, int row_sec, int *col, int *row )
{
    while ( 1 )
    {
        *col = col_sec * MAPWIN_COL_SIZE + getRand( MAPWIN_COL_SIZE );
        *row = row_sec * MAPWIN_ROW_SIZE * getRand( MAPWIN_ROW_SIZE );

        if ( is_cell_empty( *col, *row ) )
        {
            break;
        }
    }

    return;
}

void next_level( int dest_entity, int source_entity )
{
   ( void ) dest_entity;

   if ( source_entity == PLAYER_ID )
   {
      void cleanup_entities( void );
      void create_map_entities( void );

      cleanup_entities( );

      inc_level( );
      init_map( );

      create_map_entities( );

      clear_cell_entity( world.location[ source_entity ].col, world.location[ source_entity ].row );
      set_entity_col( PLAYER_ID, PLAYER_COL_START );
      set_entity_row( PLAYER_ID, PLAYER_ROW_START );
      set_cell_entity( world.location[ source_entity ].col, world.location[ source_entity ].row, source_entity );

      if ( get_level( ) < get_max_level( ) )
      {
          add_message( "You've entered sector %d.", get_level( ) + 1 );
      }
      else
      {
          add_message( "Welcome to the final level." );
      }
   }

   return;
}

void create_exit_entity( void )
{
    int entity = get_free_entity( );
    int col, row;

    get_exit_location( &col, &row );

    // Create the exit entity with an on_attack callback.
    world.id[ entity ] = entity;
    world.mask[ entity ] = COMPONENT_LOCATION | COMPONENT_RENDER;
    world.location[ entity ].col = col;
    world.location[ entity ].row = row;

    world.render[ entity ].cell = '>';
    world.render[ entity ].attr = A_BOLD;

    set_cell_entity( col, row, entity );

    world.callbacks[ entity ].on_attack = &next_level;

    return;
}

void create_planet_entity( int col, int row, int resources )
{
    int entity = get_free_entity( );
    
    // Create a planet entity
    world.id[ entity ] = entity;
    world.mask[ entity ] = COMPONENT_LOCATION | COMPONENT_RENDER | COMPONENT_RESOURCES | COMPONENT_PLANET;

    world.resources[ entity ].value = resources;

    world.location[ entity ].col = col;
    world.location[ entity ].row = row;

    set_cell_entity( col, row, entity );

    world.render[ entity ].cell = 'P';
    world.render[ entity ].attr = COLOR_PAIR( COLOR_PLANET ) | A_BOLD;

    return;
}

void demote_planet_entity( int entity )
{
    world.render[ entity ].attr = COLOR_PAIR( COLOR_PLANET ) | A_DIM;
}

void create_academy_entity( int col, int row )
{
    int entity = get_free_entity( );
    
    // Create an academy entity
    world.id[ entity ] = entity;
    world.mask[ entity ] = COMPONENT_LOCATION | COMPONENT_RENDER | COMPONENT_ACADEMY; // | COMPONENT_FRIENDLY;

    world.resources[ entity ].value = 0;

    world.location[ entity ].col = col;
    world.location[ entity ].row = row;

    set_cell_entity( col, row, entity );

    world.render[ entity ].cell = 'A';
    world.render[ entity ].attr = COLOR_PAIR( COLOR_ACADEMY ) | A_BOLD;

    return;
}

void create_wreck_entity( int col, int row, int resources )
{
    int entity = get_free_entity( );
    
    // Create a wreck entity
    world.id[ entity ] = entity;
    world.mask[ entity ] = COMPONENT_LOCATION | COMPONENT_RENDER | COMPONENT_RESOURCES | COMPONENT_WRECK;

    world.resources[ entity ].value = resources;

    world.location[ entity ].col = col;
    world.location[ entity ].row = row;

    set_cell_entity( col, row, entity );

    world.render[ entity ].cell = '%';
    world.render[ entity ].attr = COLOR_PAIR( COLOR_WRECK ) | A_BOLD;
    world.render[ entity ].attr_dec = 0;

    return;
}

void demote_wreck_entity( int entity )
{
    world.render[ entity ].attr = COLOR_PAIR( COLOR_WRECK ) | A_DIM;
}

void create_cedrone_entity( cdrone_type_t type, int col, int row, 
                            int resources, int health, int speed, int attack, int attack_speed, 
                            int armor, int xp )
{
    int entity;

    (void)xp;

    entity = get_free_entity( );
    
    // Create an enemy combat drone entity
    world.id[ entity ] = entity;
    world.mask[ entity ] = COMPONENT_ENEMY  | COMPONENT_CDRONE   | COMPONENT_HEALTH   | COMPONENT_LOCATION | 
                           COMPONENT_TARGET | COMPONENT_MOVEMENT | COMPONENT_BEHAVIOR | COMPONENT_RENDER;

    world.cdrone[ entity ].type = type;

    world.location[ entity ].col = col;
    world.location[ entity ].row = row;

    world.health[ entity ].value = health;
    world.health[ entity ].max_health = health;

    world.attack[ entity ].value = attack;
    world.attack[ entity ].armor = armor;
    world.attack[ entity ].attack_speed = attack_speed;
    world.attack[ entity ].attack_speed_state = attack_speed;

    world.movement[ entity ].speed = speed;
    world.movement[ entity ].state = speed;

    world.target[ entity ].type = TYPE_NO_TARGET;

    world.health[ entity ].value = health;
    world.health[ entity ].max_health = health;

    world.xp[ entity ].level = get_level( ) + 1;

    world.resources[ entity ].value = resources;

    world.behavior[ entity ].state = Dormant;
    world.behavior[ entity ].behavior = enemy_combat_behavior;

    set_cell_entity( world.location[ entity ].col, world.location[ entity ].row, entity );

    if ( type == Tank )
    {
        set_entity_name_str( entity, "Tank      " );
        world.render[ entity ].cell = 'T';
    }
    else if ( type == Raven )
    {
        set_entity_name_str( entity, "Raven     " );
        world.render[ entity ].cell = 'R';
    }
    else if ( type == Nova )
    {
        set_entity_name_str( entity, "Nova      " );
        world.render[ entity ].cell = 'N';
    }
    else if ( type == Kestral )
    {
        set_entity_name_str( entity, "Kestral   " );
        world.render[ entity ].cell = 'K';
    }
    else if ( type == Hunter )
    {
        set_entity_name_str( entity, "Hunter    " );
        world.render[ entity ].cell = 'H';
    }

    world.render[ entity ].attr = 0;
    world.render[ entity ].attr = COLOR_PAIR( COLOR_E_ENTITY );
    world.render[ entity ].attr_dec = 0;

    return;
}

void create_javelin_entity( int entity, int health, int speed, int attack, int armor )
{
    // Create a javelin combat drone entity
    world.id[ entity ] = entity;
    world.mask[ entity ] = COMPONENT_FRIENDLY | COMPONENT_CDRONE | COMPONENT_LOCATION | 
                           COMPONENT_HEALTH   | COMPONENT_RENDER | COMPONENT_MOVEMENT | COMPONENT_DOCKED;

    world.cdrone[ entity ].type = Javelin;

    world.location[ entity ].col = 0;
    world.location[ entity ].row = 0;

    world.health[ entity ].value = health;
    world.health[ entity ].max_health = health;

    world.attack[ entity ].value = attack;
    world.attack[ entity ].armor = armor;
    world.attack[ entity ].attack_speed = 1;
    world.attack[ entity ].attack_speed_state = 1;

    world.movement[ entity ].speed = speed;
    world.movement[ entity ].state = speed;

    world.target[ entity ].type = TYPE_NO_TARGET;

    world.health[ entity ].value = health;
    world.health[ entity ].max_health = health;

    world.xp[ entity ].level = get_level( ) + 1;

    world.behavior[ entity ].state = Dormant;
    world.behavior[ entity ].behavior = javelin_behavior;

    set_entity_name_str( entity, "Javelin   " );
    world.render[ entity ].cell = 'J';

    set_entity_state_str( entity, "Docked    " );

    world.render[ entity ].attr = COLOR_PAIR( COLOR_F_ENTITY );
    world.render[ entity ].attr_dec = 0;

    return;
}

void clear_target_entities( int target_entity )
{
    for ( int i = 1 ; i < MAX_ENTITIES ; i++ )
    {
        if ( world.target[ i ].entity_id == target_entity )
        {
            clear_entity_target( i );
            stop_entity_behavior( i );
        }
    }
}

char *side( int entity )
{
    if ( world.mask[ entity ] & COMPONENT_FRIENDLY ) return "Friendly";
    return "Enemy";
}

void damage( int entity, int target_entity, int damage )
{
    if ( is_friendly( target_entity ) ) 
    {
        set_entity_render_alternate_attr( target_entity, COLOR_PAIR( COLOR_F_UNDER_ATTACK ), 20 );
        add_damage_received( damage );
    }
    else 
    {
        set_entity_render_alternate_attr( target_entity, COLOR_PAIR( COLOR_E_UNDER_ATTACK ), 20 );
        add_damage_dealt( damage );
        add_xp( entity, damage );
    }

    add_message( "%s %s Hits %s %s for %d damage.", side( entity ), get_entity_name_str( entity ), 
                   side( target_entity ), get_entity_name_str( target_entity ), damage );

    int health = get_entity_health( target_entity ) - damage;
    set_entity_health( target_entity, health );

    if ( health <= 0 )
    {
        if ( target_entity == PLAYER_ID )
        {
            set_death_type( Killed );
            end_game( );
        }
        else
        {
            add_message( "%s %s has been destroyed", ( is_friendly( target_entity ) ? "Friendly" : "Enemy" ),
                            get_entity_name_str( target_entity ) );

            if ( is_friendly( target_entity ) ) add_friendly_drones_destroyed( 1 );
            else add_enemy_drones_destroyed( 1 );

            int col = get_entity_col( target_entity );
            int row = get_entity_row( target_entity );
            clear_cell_entity( col, row );

            destroy_entity( target_entity );
            clear_entity_target( entity );

            create_wreck_entity( col, row, ( get_wreck_resources( get_level( ) ) + 2 ) );
            add_xp( entity, DRONE_KILL_XP );

            clear_target_entities( target_entity );

            if ( is_enemy( entity ) )
            {
                world.behavior[ entity ].state = Dormant;
                start_entity_behavior( entity );
            }
        }
    }
}

void javelin_explosion( int entity )
{
    int radius = 5;

    clear_entity_target( entity );
    int col = get_entity_col( entity );
    int row = get_entity_row( entity );
    create_explosion( col, row, radius );

    for ( int index = 0 ; index < MAX_ENTITIES ; index++ )
    {
        if ( world.mask[ index ] & COMPONENT_RENDER )
        {
            if ( world.mask[ index ] & COMPONENT_HEALTH )
            {
                int distance = entity_distance( entity, index );
                if ( distance <= radius )
                {
                    int real_damage = ( world.attack[ entity ].value - distance );
                    damage( entity, index, real_damage );
                }
            }
        }
    }

    destroy_entity( entity );
}

void set_entity_docked( int entity )
{
    world.mask[ entity ] |= COMPONENT_DOCKED;
}

void reset_entity_docked( int entity )
{
    world.mask[ entity ] &= ~COMPONENT_DOCKED;
}

bool is_entity_docked( int entity )
{
    return world.mask[ entity ] & COMPONENT_DOCKED;
}

bool is_entity_cdrone( int entity )
{
    return world.mask[ entity ] & COMPONENT_CDRONE;
}

bool is_entity_friendly( int entity )
{
    return world.mask[ entity ] & COMPONENT_FRIENDLY;
}

bool is_entity_enemy( int entity )
{
    return world.mask[ entity ] & COMPONENT_ENEMY;
}

bool is_entity_morphing( int entity )
{
    return world.mask[ entity ] & COMPONENT_MORPH;
}

bool is_entity_miner( int entity )
{
    return ( world.edrone[ entity ].type == Miner );
}

bool is_entity_scavenger( int entity )
{
    return ( world.edrone[ entity ].type == Scavenger );
}

void start_entity_behavior( int entity )
{
    world.behavior[ entity ].state = Dormant;
    world.mask[ entity ] |= COMPONENT_BEHAVIOR;
}

void stop_entity_behavior( int entity )
{
    world.behavior[ entity ].state = Dormant;
    world.mask[ entity ] &= ~COMPONENT_BEHAVIOR;
}

static void create_edrone_entity( int entity, edrone_type_t type, int health, int speed, int max_resources, int scav_speed )
{
    // Create a scaveger drone entity
    world.id[ entity ] = entity;
    world.mask[ entity ] = COMPONENT_FRIENDLY | COMPONENT_EDRONE | COMPONENT_LOCATION | 
                           COMPONENT_HEALTH   | COMPONENT_RENDER | COMPONENT_MOVEMENT | COMPONENT_DOCKED;

    // COMPONENT_RENDER is added once the drone is undocked.

    world.edrone[ entity ].type = type;

    world.location[ entity ].col = 0;
    world.location[ entity ].row = 0;

    world.health[ entity ].value = health;
    world.health[ entity ].max_health = health;

    world.attack[ entity ].value = 0;
    world.attack[ entity ].armor = 2;

    world.movement[ entity ].speed = speed;
    world.movement[ entity ].state = speed;

    world.target[ entity ].type = TYPE_NO_TARGET;

    world.health[ entity ].value = health;
    world.health[ entity ].max_health = health;

    world.xp[ entity ].level = 1;

    world.resources[ entity ].value = 0;

    world.edrone[ entity ].max_resources = max_resources;
    world.edrone[ entity ].extract_speed = scav_speed;
    world.edrone[ entity ].extract_state = 0;

    world.behavior[ entity ].state = Dormant;
    world.behavior[ entity ].behavior = extract_behavior;

    set_cell_entity( world.location[ entity ].col, world.location[ entity ].row, entity );

    if ( type == Scavenger )
    {
        set_entity_name_str( entity, "Scavenger " );
        world.render[ entity ].cell = 'S';
    }
    else if ( type == Miner )
    {
        set_entity_name_str( entity, "Miner     " );
        world.render[ entity ].cell = 'M';
    }
    world.render[ entity ].attr = 0;
    world.render[ entity ].attr_dec = 0;

    set_entity_state_str( entity, "Docked    " );

    return;
}

bool get_entity_render( int entity, chtype *ch )
{
    bool result = false;

    if ( world.mask[ entity ] & COMPONENT_RENDER )
    {
        if ( world.render[ entity ].attr_dec == 0 )
        {
            *ch = world.render[ entity ].cell | world.render[ entity ].attr;
        }
        else
        {
            world.render[ entity ].attr_dec--;
            *ch = world.render[ entity ].cell | world.render[ entity ].attr_alternate;
        }
        result = true;
    }
    else
    {
        *ch = ' ';
    }

    return result;
}

void set_entity_render_alternate_attr( int entity, chtype ch, int count )
{
    world.render[ entity ].attr_alternate = ch;
    world.render[ entity ].attr_dec = count;
}

void create_player_entity( )
{
    int entity;

    entity = 0; // get_free_entity( );

    // Create the player entity
    world.id[ entity ] = entity;
    world.mask[ entity ] = COMPONENT_LOCATION | COMPONENT_HEALTH | COMPONENT_MOVEMENT | 
                           COMPONENT_RENDER | COMPONENT_FRIENDLY | COMPONENT_PLAYER;

    world.location[ entity ].col = PLAYER_COL_START;
    world.location[ entity ].row = PLAYER_ROW_START;

    world.xp[ entity ].level = 1;

    world.health[ entity ].value = 2;
    world.health[ entity ].max_health = 2;

    world.attack[ entity ].value = 0;
    world.attack[ entity ].armor = 2;

    world.movement[ entity ].speed = PLAYER_MOVE_SPEED;
    world.movement[ entity ].state = PLAYER_MOVE_SPEED;

    world.resources[ entity ].value = INITIAL_PLAYER_RESOURCES;

    world.render[ entity ].cell = '@';
    world.render[ entity ].attr = COLOR_PAIR( COLOR_BORG ) | A_BOLD;
    world.render[ entity ].attr_dec = 0;

    set_cell_entity( world.location[ entity ].col, world.location[ entity ].row, entity );

    set_entity_name_str( entity, "Borg[@]   " );
    set_entity_state_str( entity, "Operating " );

    return;
}

void create_boss_entity( )
{
    int entity;
    int col, row;

    entity = get_free_entity( );

    // Create the player entity
    world.id[ entity ] = entity;
    world.mask[ entity ] = COMPONENT_LOCATION | COMPONENT_HEALTH | COMPONENT_BEHAVIOR | 
                           COMPONENT_RENDER   | COMPONENT_ENEMY;

    get_exit_location( &col, &row );
    // TODO
    world.location[ entity ].col = 240;
//    world.location[ entity ].col = col - 2;
    world.location[ entity ].row = row;

    world.xp[ entity ].level = 4;

    world.health[ entity ].value = 20;
    world.health[ entity ].max_health = 20;

    world.attack[ entity ].value = 0;
    world.attack[ entity ].armor = 3;

    world.resources[ entity ].value = 0;

    world.render[ entity ].cell = 'B';
    world.render[ entity ].attr = COLOR_PAIR( COLOR_E_ENTITY ) | A_BOLD;
    world.render[ entity ].attr_dec = 0;

    world.behavior[ entity ].state = Dormant;
    world.behavior[ entity ].behavior = boss_behavior;

    set_entity_name_str( entity, "Boss      " );

    set_cell_entity( world.location[ entity ].col, world.location[ entity ].row, entity );

    return;
}

int get_entity_col( int entity )
{
    return world.location[ entity ].col;
}

int get_entity_row( int entity )
{
    return world.location[ entity ].row;
}

int get_entity_resources( int entity )
{
    return world.resources[ entity ].value;
}

void set_entity_resources( int entity, int value )
{
    world.resources[ entity ].value = value;
}

int get_entity_attack( int entity )
{
    return world.attack[ entity ].value;
}

int get_entity_armor( int entity )
{
    return world.attack[ entity ].armor;
}

int get_entity_level( int entity )
{
    return world.xp[ entity ].level;
}

int get_entity_max_health( int entity )
{
    return world.health[ entity ].max_health;
}

int get_entity_health( int entity )
{
    return world.health[ entity ].value;
}

void set_entity_health( int entity, int value )
{
    world.health[ entity ].value = value;
}

bool decrement_heal_speed_cur( int entity )
{
    if ( world.heal[ entity ].heal_speed_cur == 0 )
    {
        world.heal[ entity ].heal_speed_cur = MAX_HEAL_SPEED;
        return true;
    }
    else
    {
        world.heal[ entity ].heal_speed_cur--;
    }

    return false;
}

void set_entity_col( int entity, int col )
{
    world.location[ entity ].col = col;
}

void set_entity_row( int entity, int row )
{
    world.location[ entity ].row = row;
}

void start_entity_healing( int entity )
{
    world.mask[ entity ] |= COMPONENT_HEAL;
    world.heal[ entity ].heal_speed_cur = MAX_HEAL_SPEED;
    set_entity_state_str( entity, "Healing   " );
}

void stop_entity_healing( int entity )
{
    world.mask[ entity ] &= ~COMPONENT_HEAL;
    set_entity_state_str( entity, "Docked    " );
}

void set_morph_data( int entity, int delay, char type )
{
    world.morph[ entity ].result = type;
    world.morph[ entity ].morph_completion = delay;

    set_entity_state_str( entity, "Morphing  " );

    world.mask[ entity ] |= COMPONENT_MORPH;
}

behavior_state_t get_entity_behavior_state( int entity )
{
    return world.behavior[ entity ].state;
}

void set_entity_behavior_state( int entity, behavior_state_t state )
{
    world.behavior[ entity ].state = state;
}

int get_edrone_max_resources( int entity )
{
    return world.edrone[ entity ].max_resources;
}

int get_edrone_speed( int entity )
{
    return world.edrone[ entity ].extract_speed;
}

int get_edrone_state( int entity )
{
    return world.edrone[ entity ].extract_state;
}

void set_edrone_state( int entity, int value )
{
    world.edrone[ entity ].extract_state = value;
}

void set_entity_state_str( int entity, char *state )
{
    strcpy( world.estate[ entity ].state_str, state );
}

char* get_entity_state_str( int entity )
{
    return world.estate[ entity ].state_str;
}

void set_entity_name_str( int entity, char *name )
{
    strcpy( world.ename[ entity ].name_str, name );
}

char *get_entity_name_str( int entity )
{
    return world.ename[ entity ].name_str;
}

char *get_edrone_action_str( int entity )
{
    switch( world.edrone[ entity ].type )
    {
        case Scavenger:
            return "Scavenging"; break;
        case Miner:
            return "Mining    "; break;
        default:
            return "Unknown   "; break;
    }
}

bool is_friendly( int entity )
{
    if ( world.mask[ entity ] & COMPONENT_FRIENDLY ) return true;
    return false;
}

bool is_enemy( int entity )
{
    if ( world.mask[ entity ] & COMPONENT_ENEMY ) return true;
    return false;
}

bool is_wreck( int entity )
{
    if ( world.mask[ entity ] & COMPONENT_WRECK ) return true;
    return false;
}

bool is_planet( int entity )
{
    if ( world.mask[ entity ] & COMPONENT_PLANET ) return true;
    return false;
}

bool is_academy( int entity )
{
    if ( world.mask[ entity ] & COMPONENT_ACADEMY ) return true;
    return false;
}

bool is_friendly_combat_drone( int entity )
{
    if ( world.mask[ entity ] & COMPONENT_FRIENDLY )
    {
        if ( world.mask[ entity ] & COMPONENT_CDRONE )
        {
            if ( ( world.mask[ entity ] & COMPONENT_DOCKED ) == 0 )
            return true;
        }
    }

    return false;
}

bool is_target_compatible( int entity, int target )
{
    if      ( ( world.edrone[ entity ].type == Scavenger ) && ( is_wreck( target ) ) ) return true;
    else if ( ( world.edrone[ entity ].type == Miner )     && ( is_planet( target ) ) ) return true;
    else if ( ( entity == PLAYER_ID )                      && ( world.render[ target ].cell = '>' ) ) return true;
    else if ( ( entity == PLAYER_ID )                      && ( is_academy( target ) ) ) return true;
    else if ( ( is_friendly( entity )                      && ( is_academy( target ) ) ) ) return true;
    else if ( ( is_friendly( entity )                      && ( is_enemy( target ) ) ) ) return true;
    else if ( ( is_enemy( entity )                         && ( is_friendly( target ) ) ) ) return true;
    else if ( ( is_friendly_combat_drone( entity )         && ( is_wreck( target ) ) ) ) return false;
    else if ( ( is_friendly_combat_drone( entity )         && ( is_planet( target ) ) ) ) return false;

    return false;
}

static void place_wreck( int col, int row )
{
    int coff, roff;
    do
    {
        coff = getRand( MAPWIN_COL_SIZE );
        roff = getRand( MAPWIN_ROW_SIZE );
    }
    while( !is_cell_empty( col+coff, row+roff ) );

    create_wreck_entity( col+coff, row+roff, get_wreck_resources( get_level( ) ) );

    return;
}

static void place_planet( int col, int row )
{
    int coff, roff;
    do
    {
        coff = getRand( MAPWIN_COL_SIZE );
        roff = getRand( MAPWIN_ROW_SIZE );
    }
    while( !is_cell_empty( col+coff, row+roff ) );

    create_planet_entity( col+coff, row+roff, get_planet_resources( get_level( ) ) );

    return;
}

static void place_academy( int col, int row )
{
    int coff, roff;
    do
    {
        coff = getRand( MAPWIN_COL_SIZE );
        roff = getRand( MAPWIN_ROW_SIZE );
    }
    while( !is_cell_empty( col+coff, row+roff ) );

    create_academy_entity( col+coff, row+roff );

    return;
}

static void place_combat_drone( int col, int row, int index )
{
    int coff, roff;

    do
    {
        coff = getRand( MAPWIN_COL_SIZE );
        roff = getRand( MAPWIN_ROW_SIZE );
    }
    while( !is_cell_empty( col+coff, row+roff ) );

    int level = get_level( );

    create_cedrone_entity( get_cdrone_type( get_level( ), index ), col+coff, row+roff, 0, 
                            get_cd_hp( level, index ), get_cd_move_speed( level, index ), 
                            get_cd_attack( level, index ), get_cd_attack_speed( level, index ), 
                            get_cd_armor( level, index ), get_cd_hp( level, index ) );
    return;
}

void create_map_entities( void )
{
    // Create wrecks in the environment.
    int wrecks = get_wreck_count( get_level( ) );

    while ( wrecks )
    {
        int sector_col = getRand( MAP_SEC_NCOLS );
        int sector_row = getRand( MAP_SEC_NROWS );

        place_wreck( sector_col * MAPWIN_COL_SIZE, sector_row * MAPWIN_ROW_SIZE );
        wrecks--;
    }

    // Create planets in the environment.
    int planets = get_planet_count( get_level( ) );

    while ( planets )
    {
        int sector_col = getRand( MAP_SEC_NCOLS );
        int sector_row = getRand( MAP_SEC_NROWS );

        place_planet( sector_col * MAPWIN_COL_SIZE, sector_row * MAPWIN_ROW_SIZE );
        planets--;
    }

    // Create academies.
    int academies = get_academies( get_level( ) );

    while ( academies )
    {
        int sector_col = getRand( MAP_SEC_NCOLS-1 ) + 1;
        int sector_row = getRand( MAP_SEC_NROWS );

        place_academy( sector_col * MAPWIN_COL_SIZE, sector_row * MAPWIN_ROW_SIZE );
        academies--;
    }

    for ( int index = 0 ; index < MAX_DRONE_TYPES ; index++ )
    {
        // Create combat drones in the environment
        int cdrones = get_combat_drones( get_level( ), index );

        while ( cdrones )
        {
            int sector_col = getRand( MAP_SEC_NCOLS-1 ) + 1;
            int sector_row = getRand( MAP_SEC_NROWS );

            place_combat_drone( sector_col * MAPWIN_COL_SIZE, sector_row * MAPWIN_ROW_SIZE, index );
            cdrones--;
        }
    }

    // Create map exit entity.
    if ( get_level( ) < get_max_level( ) )
    {
        create_exit_entity( );
        create_boss_entity( );
    }
    else
    {
        create_boss_entity( );
    }

    return;
}

// Remove all entities that are not player and friendly.
void cleanup_entities( void )
{
    // Skip offset 0 which is the player (Borg)
    for ( int entity = 1 ; entity < MAX_ENTITIES ; entity++ )
    {
        if ( world.mask[ entity ] & COMPONENT_FRIENDLY )
        {
            if ( world.mask[ entity ] & COMPONENT_DOCKED )
            {
                continue;
            }
        }

        destroy_entity( entity );
    }

    return;
}

static void create_entities( void )
{
    if ( get_level( ) == 0 )
    {
        // Player is first (entity 0).
        create_player_entity( );
        create_edrone_entity( get_free_entity( ), Scavenger, 2, SCAVENGER_MOVE_SPEED, 4, SCAV_SPEED );
        create_edrone_entity( get_free_entity( ), Miner,     3, MINER_MOVE_SPEED,     5, MINE_SPEED );
    }

    create_map_entities( );

    return;
}

void init_entities( void )
{
    int entity;

    for ( entity = 0 ; entity < MAX_ENTITIES ; entity++ )
    {
        destroy_entity( entity );
    }

    create_entities( );

    return;
}

bool get_player_inv( int entity, char *object, char *state, int *lvl, int *hp, int *max_hp, int *armor, int *attack, int *resources )
{
    bool status = false;

    *lvl = world.xp[ entity ].level;
    *hp  = world.health[ entity ].value;
    *max_hp = world.health[ entity ].max_health;
    *armor = world.attack[ entity ].armor;
    *attack = world.attack[ entity ].value;
    *resources = world.resources[ entity ].value;

    if ( world.mask[ entity ] != COMPONENT_NONE )
    {
        if ( world.mask[ entity ] & COMPONENT_FRIENDLY )
        {
            if ( ( world.mask[ entity ] & COMPONENT_EDRONE ) || 
                 ( world.mask[ entity ] & COMPONENT_CDRONE ) )
            {
                strcpy( object, get_entity_name_str( entity ) );
                strcpy( state, get_entity_state_str( entity ) );

                status = true;
            }
            else if ( world.mask[ entity ] & COMPONENT_PLAYER )
            {
                strcpy( object, "Borg [@]  " );
                strcpy( state, "Operating " );
    
                status = true;
            }
        }
    }

    return status;
}

void set_target_loc( int entity, cmovement_type_t cmove, int col, int row )
{
    if ( world.mask[ entity ] & COMPONENT_MOVEMENT )
    {
        int start_col = world.location[ entity ].col;
        int start_row = world.location[ entity ].row;
        int dcol, drow;

        // First, see if there's a path there.
        if ( MoveTowardsTarget( start_row, start_col, row, col, &drow, &dcol ) )
        {
            world.target[ entity ].type = cmove;
            world.target[ entity ].entity_id = -1;
            world.target[ entity ].col = col;
            world.target[ entity ].row = row;
            set_cell_ephemeral( col, row, 60, COLOR_PAIR(COLOR_TRAIL) | '*' );
        }
        else
        {
           if ( entity == PLAYER_ID ) add_message( "Can't move there" );
           world.target[ entity ].type = TYPE_NO_TARGET;
        }
    }

    return;
}

void set_target_entity( int entity, cmovement_type_t cmove, int target_entity )
{
    if ( world.mask[ entity ] & COMPONENT_MOVEMENT )
    {
        int start_col = world.location[ entity ].col;
        int start_row = world.location[ entity ].row;
        int col = world.location[ target_entity ].col;
        int row = world.location[ target_entity ].row;
        int dcol, drow;

        // First, see if there's a path there.
        if ( MoveTowardsTarget( start_row, start_col, row, col, &drow, &dcol ) )
        {
            world.target[ entity ].type = cmove;
            world.target[ entity ].entity_id = target_entity;
            set_cell_ephemeral( col, row, 60, COLOR_PAIR(COLOR_TRAIL) | '*' );
        }
        else
        {
           if ( entity == PLAYER_ID ) add_message( "Can't move there" );
           world.target[ entity ].type = TYPE_NO_TARGET;
        }
    }

    return;
}

void set_target_dock( int entity, cmovement_type_t cmove, int target_entity )
{
    if ( !is_entity_docked( entity ) )
    {
        if ( world.mask[ entity ] & COMPONENT_MOVEMENT )
        {
            int start_col = world.location[ entity ].col;
            int start_row = world.location[ entity ].row;
            int col = world.location[ target_entity ].col;
            int row = world.location[ target_entity ].row;
            int dcol, drow;

            // First, see if there's a path there.
            if ( MoveTowardsTarget( start_row, start_col, row, col, &drow, &dcol ) )
            {
                world.target[ entity ].type = cmove;
                world.target[ entity ].entity_id = target_entity;
                set_cell_ephemeral( col, row, 60, COLOR_PAIR(COLOR_TRAIL) | '*' );
                set_entity_state_str( entity, "Docking   " );
            }
            else
            {
               if ( entity == PLAYER_ID ) add_message( "Can't dock from here." );
               world.target[ entity ].type = TYPE_NO_TARGET;
            }
        }
    }

    return;
}

void set_entity_target( int entity, int target_entity )
{
    world.target[ entity ].entity_id = target_entity;
}

void clear_entity_target( int entity )
{
    world.target[ entity ].type = TYPE_NO_TARGET;
//    world.target[ entity ].entity_id = NO_ENTITY;
//    world.target[ entity ].cached_entity_id = NO_ENTITY;
}

void set_cached_entity_target( int entity, int target_entity )
{
    world.target[ entity ].cached_entity_id = target_entity;
}

int get_cached_entity_target( int entity )
{
    return world.target[ entity ].cached_entity_id;
}

void entity_move( int entity, int cold, int rowd )
{
   location_t loc;

   // Get Entity Location
   loc.col = get_entity_col( entity );
   loc.row = get_entity_row( entity );

   // First, check to see if an entity is in the new location.
   int target_entity = get_entity_at( cold, rowd );

   // If the next step is the target, time to dock.
   if ( target_entity == PLAYER_ID && world.target[ entity ].type == TYPE_ENTITY_DOCK )
   {
      // Dock the entity
      world.target[ entity ].type = TYPE_NO_TARGET;
      set_entity_docked( entity );
      clear_cell_entity( loc.col, loc.row );
      clear_entity_target( entity );

      // If the entity has resources, move them to the Borg.
      set_entity_resources( PLAYER_ID, ( get_entity_resources( PLAYER_ID ) + get_entity_resources( entity ) ) );
      set_entity_resources( entity, 0 );

      // Remove the entity from space.
      set_entity_col( entity, -1 );
      set_entity_row( entity, -1 );
      set_entity_state_str( entity, "Docked    " );
      return;
   }
   else if ( target_entity != NO_ENTITY )
   {
      // TODO: if both friendly, or both enemy, ignore this...
      if ( world.callbacks[ target_entity ].on_attack != ( on_verb_callback) 0 )
      {
         (world.callbacks[ target_entity ].on_attack)( target_entity, entity );
      }

      return;
   }

   if ( !passable( cold, rowd ) )
   {
      return;
   }

   clear_cell_entity( loc.col, loc.row );

   // Create the trail of the entity.
   set_cell_ephemeral( loc.col, loc.row, 15, COLOR_PAIR( COLOR_TRAIL ) | A_DIM | '.' );

   loc.row = rowd;
   loc.col = cold;

   if ( loc.row < 0 )
   {
      loc.row = 0;
   }
   else if ( loc.col < 0 )
   {
      loc.col = 0;
   }
   else if ( loc.row >= MAP_MAX_NROWS )
   {
      loc.row = MAP_MAX_NROWS - 1;
   }
   else if ( loc.col >= MAP_MAX_NCOLS )
   {
      loc.col = MAP_MAX_NCOLS - 1;
   }

   set_entity_col( entity, loc.col );
   set_entity_row( entity, loc.row );

   set_cell_entity( loc.col, loc.row, entity );

   return;
}


int entity_distance( int entity, int target_entity )
{
   #define SQ( x )    ( (x) * (x) )
   int x1 = get_entity_col( entity ) - get_entity_col( target_entity );
   int x2 = get_entity_row( entity ) - get_entity_row( target_entity );

   return (int) sqrt( ( double ) ( SQ( x1 ) + SQ( x2 ) ) );
}

int find_close_entity( int entity )
{
   int target_entity = NO_ENTITY;

   for ( int i = 0 ; i < MAX_ENTITIES ; i++ )
   {
      if ( world.mask[ i ] & COMPONENT_FRIENDLY )
      {
         if ( ( world.mask[ i ] & COMPONENT_DOCKED ) == 0 )
         {
            if ( entity_distance( entity, i ) < MAX_RADAR_DISTANCE )
            {
               target_entity = i;
               if ( getSRand( ) > 0.5 ) break;
            }
         }
      }
   }

   return target_entity;
}

void academy_edrone_upgrade( int entity )
{
   // Expose the options for upgrading an extractor drone at the academy.
   char *array[3] = {
         "Upgrade Drone Armor",
         "Upgrade Drone Health",
         "Increase Extraction Speed"
      };

   int selection = emit_menu( "Academy", array, 3 );

   if ( world.resources[ PLAYER_ID ].value >= UPGRADE_COST )
   {
       if ( selection == 0 )
       {
            world.attack[ entity ].armor++;
            add_message( "Upgraded drone armor +1" );
            world.resources[ PLAYER_ID ].value -= UPGRADE_COST;
       }
       else if ( selection == 1 )
       {
            world.health[ entity ].max_health += 1;
            world.health[ entity ].value = world.health[ entity ].max_health;
            add_message( "Upgraded drone health +1" );
            world.resources[ PLAYER_ID ].value -= UPGRADE_COST;
       }
       else if ( selection == 2 )
       {
            world.edrone[ entity ].extract_speed -= 5;
            world.edrone[ entity ].extract_state = world.edrone[ entity ].extract_speed;
            add_message( "Increased Extraction Speed" );
            world.resources[ PLAYER_ID ].value -= UPGRADE_COST;
       }
   }
   else
   {
      add_message( "Insufficient resources for extractor drone upgrade (%d resources).", UPGRADE_COST );
   }

}

void academy_cdrone_upgrade( int entity )
{
   // Expose the options for upgrading an extractor drone at the academy.
   char *array[4] = {
         "Upgrade Drone Armor",
         "Upgrade Drone Attack",
         "Upgrade Drone Health",
         "Increase Attack Speed"
      };

   int selection = emit_menu( "Academy", array, 4 );

   if ( world.resources[ PLAYER_ID ].value >= UPGRADE_COST )
   {
       if ( selection == 0 )
       {
            world.attack[ entity ].armor++;
            add_message( "Upgraded drone armor +1" );
            world.resources[ PLAYER_ID ].value -= UPGRADE_COST;
       }
       else if ( selection == 1 )
       {
            world.attack[ entity ].value++;
            add_message( "Upgraded drone attack +1" );
            world.resources[ PLAYER_ID ].value -= UPGRADE_COST;
       }
       else if ( selection == 2 )
       {
            world.health[ entity ].max_health += 1;
            world.health[ entity ].value = world.health[ entity ].max_health;
            add_message( "Upgraded drone health +1" );
            world.resources[ PLAYER_ID ].value -= UPGRADE_COST;
       }
       else if ( selection == 3 )
       {
            world.attack[ entity ].attack_speed -= 5;
            world.attack[ entity ].attack_speed_state = world.attack[ entity ].attack_speed;
            add_message( "Increased Attack Speed" );
            world.resources[ PLAYER_ID ].value -= UPGRADE_COST;
       }
   }
   else
   {
      add_message( "Insufficient resources for combat drone upgrade." );
   }
}


// TODO: Based upon drone type, focus updates that matter.

void get_random_academy_upgrade( int entity, bool dec_resources )
{
    int extra = (int)( getSRand( ) >= 0.97 );
    int value = 1 + extra;

    int taken = 0;

    while ( !taken )
    {
        int upgrade = getRand( 3 );

        switch( upgrade )
        {
            case 0: // Attack
                if ( world.mask[ entity ] & COMPONENT_CDRONE ) 
                {
                    world.attack[ entity ].value++;
                    add_message( "Upgraded attack +%d", 1 );
                    taken = 1;
                }
                break;

            case 1: // Armor
                world.attack[ entity ].armor++;
                add_message( "Upgraded armor +%d", 1 );
                taken = 1;
                break;

            case 2: // Health
                world.health[ entity ].max_health += value;
                world.health[ entity ].value = world.health[ entity ].max_health;
                add_message( "Upgraded health +%d", value );
                taken = 1;
                break;
        }

    }

    if ( dec_resources )
    {
        // Decrement resources.
        set_entity_resources( PLAYER_ID, get_entity_resources( PLAYER_ID ) - UPGRADE_COST );
    }

    return;
}

void receive_random_upgrade( void )
{
    add_message( "You've received a random upgrade." );

    int upgrade = getRand( 3 );

    if ( upgrade == 1 )
    {
        create_edrone_entity( get_free_entity( ), Scavenger, 2, 10, 3, 70 );
        add_message( "   An extra scavenger drone." );
    }
    else if ( upgrade == 2 )
    {
        create_edrone_entity( get_free_entity( ), Miner, 3, 20, 4, 90 );
        add_message( "   An extra miner drone." );
    }
    else
    {
        int resources = RANDOM_RES_UPGRADE;
        add_message( "   +%d resources. ", resources );
        set_entity_resources( PLAYER_ID, get_entity_resources( PLAYER_ID ) + resources );
    }

    return;
}

void emit_map_context_info( int entity )
{
    char line[ CONTEXTWIN_COL_SIZE + 1 ];

    if ( world.mask[ entity ] & COMPONENT_ACADEMY )
    {
        strncpy( line, "Academy for drone upgrades.", CONTEXTWIN_COL_SIZE );
    }
    else if ( world.mask[ entity ] & COMPONENT_WRECK )
    {
        snprintf( line, CONTEXTWIN_COL_SIZE, 
                    "Abandoned wreck.  Contains %d resources for scavenging.", get_entity_resources( entity ) );
    }
    else if ( world.mask[ entity ] & COMPONENT_PLANET )
    {
        snprintf( line, CONTEXTWIN_COL_SIZE, 
                    "Isolated planet.  Contains %d resources for mining.", get_entity_resources( entity ) );
    }
    else if ( ( world.mask[ entity ] & COMPONENT_CDRONE ) || ( world.mask[ entity ] & COMPONENT_EDRONE ) )
    {
        int friendly = world.mask[ entity ] & COMPONENT_FRIENDLY;
        float p = get_assimilate_probability( PLAYER_ID, entity );
        int combat = world.mask[ entity ] & COMPONENT_CDRONE;

        // TODO: Don't offer prob assimilate for friendly drone.
        snprintf( line, CONTEXTWIN_COL_SIZE,
                    "%s %s drone.  Level %d Attack/Armor %d/%d HP %d (prob assimilate %d%%)", 
                    ( friendly ? "Friendly" : "Enemy" ), ( combat ? "combat" : "extractor" ),
                    get_entity_level( entity ), get_entity_attack( entity ), get_entity_armor( entity ),
                    get_entity_health( entity ),
                    (int)( p * 100 ) );

    }
    else
    {
        strncpy( line, "Select an object in the map and press i for information.", CONTEXTWIN_COL_SIZE );
    }

    add_message( line );
//    set_context( line );
}

void add_xp( int entity, int xpoints )
{
    world.xp[ entity ].value += xpoints;

    if ( world.xp[ entity ].value >= ( BASE_XP * pow( (float)world.xp[ entity ].level, FACTOR ) ) )
    {
        world.xp[ entity ].level++;

        add_message("%s reached level %d", world.ename[ entity].name_str, world.xp[ entity ].level );

        // Random upgrade.
        get_random_academy_upgrade( entity, false );
    }

}

void attack( int entity, int target_entity )
{
    if ( --world.attack[ entity ].attack_speed_state == 0 )
    {
        float potential;
        int attacker = world.health[ entity ].value * world.attack[ entity ].value;
        int attackee = world.health[ target_entity ].value * world.attack[ target_entity ].armor;

        world.attack[ entity ].attack_speed_state = world.attack[ entity ].attack_speed; 

        potential = ( ( 1.0 / ( float )( attacker + attackee ) ) * ( float ) attacker );

        if ( getSRand( ) < potential )
        {
            // Hit!  Compute damage...
            int real_damage = 1;

            damage( entity, target_entity, real_damage );
        }
        else
        {
            add_message( "%s %s Misses %s %s.", side( entity ), get_entity_name_str( entity ), 
                           side( target_entity ), get_entity_name_str( target_entity ) );
        }
    }
}

// TODO: Consider making assimilate a system (so that it takes a little time to perform).
void assimilate( int entity )
{
    world.mask[ entity ] &= ~( COMPONENT_ENEMY | COMPONENT_BEHAVIOR );

    world.mask[ entity ] |= COMPONENT_FRIENDLY;

    clear_entity_target( entity );

    world.behavior[ entity ].state = Dormant;
    world.behavior[ entity ].behavior = friendly_combat_behavior;

    set_entity_state_str( entity, "Undocked  " );

    world.render[ entity ].attr = COLOR_PAIR( COLOR_F_ENTITY );

    add_xp( PLAYER_ID, 2 );
    add_drones_assimilated( 1 );

    add_message( "Drone successfully assimilated." );

    return;
}

void endgame_emit( void )
{
    switch( get_death_type( ) )
    {
        case Surrendered:
            printf( "\nThe Borg has surrendered.\n\n" );
            break;
        case Killed:
            printf( "\nThe Borg has been killed.\n\n" );
            break;
        case VoidSpace:
            printf( "\nThe Borg has been trapped in void space.\n\n" );
            break;

        default:
            assert(0);
            break;
    }

    printf( "Your stats:\n\n" );
    printf( "\tDrones assimilated           : %3d\n", get_drones_assimilated( ) );
    printf( "\tEnemy drones destroyed       : %3d\n", get_enemy_drones_destroyed( ) );
    printf( "\tFriendly drones destroyed    : %3d\n", get_friendly_drones_destroyed( ) );
    printf( "\tHealing administred          : %3d\n", get_healing_administred( ) );
    printf( "\tDamage dealt to enemies      : %3d\n", get_damage_dealt( ) );
    printf( "\tDamage received from enemies : %3d\n", get_damage_received( ) );
    printf( "\tResources scavenged          : %3d\n", get_resources_scavenged( ) );
    printf( "\tResources mined              : %3d\n", get_resources_mined( ) );
    printf( "\tDrones recycled              : %3d\n", get_drones_recycled( ) );
    printf( "\tDrones recycled resources    : %3d\n", get_drones_recycled_resources( ) );
    printf( "\tJavelins fired               : %3d\n", get_javelins_fired( ) );
    printf("\n");

    printf( "Your Drones:\n\n" );
    printf( "\tDrone       State       Lvl   HP/Max  Arm  Att  Res\n" );
    printf( "\t----------  ----------  ---  -------  ---  ---  ---\n" );
    for ( int i = 0 ; i < MAX_ENTITIES ; i++ )
    {
        int lvl, hp, max_hp, armor, attack, resources;
        char object[ 11 ], state[ 11 ];
        
        if ( get_player_inv( i, object, state, &lvl, &hp, &max_hp, &armor, &attack, &resources ) )
        {
            printf( "\t%s  %s  %3d  %3d/%3d  %3d  %3d  %3d\n", object, state, lvl, hp, max_hp, armor, attack, resources );
        }
    }
    printf("\n");

    do
    {
        if ( getSRand( ) < 0.7 )
        {
            // Check possible tips for the player
            if ( get_entity_resources( PLAYER_ID ) == 0 )
            {
                printf( "\nTip: Use your scavenger and miner drones to collect resources for use.\n\n" );
                break;
            }
            else if ( get_death_type( ) == VoidSpace )
            {
                printf( "\nTip: Keep moving forward to avoid being trapped in void space.\n\n" );
                break;
            }
            else if ( get_drones_assimilated( ) == 0 )
            {
                printf( "\nTip: Assimilate enemy drones using 'a'.\n\n" );
                break;
            }
            else if ( get_resources_mined( ) == 0 )
            {
                printf( "\nTip: Use your miner drone to mine planets for resources.\n\n" );
                break;
            }
            else if ( get_resources_scavenged( ) == 0 )
            {
                printf( "\nTip: Use your scavenger drone to scavenge wrecks for resources.\n\n" );
                break;
            }
            else if ( get_javelins_fired( ) == 0 )
            {
                printf( "\nTip: Morph a combat drone into a javelin to destroy tougher enemies.\n\n" );
                break;
            }
        }

        // Pick a random tip.
        int choice = getRand( 10 ) - 1;

        switch( choice )
        {
            case 0:
                printf( "\nTip: Right-click in the map to move the Borg.\n\n" );
                break;
            case 1:
                printf( "\nTip: Left-click a drone in the inventory (or map) and Right-click the map to move.\n\n" );
                break;
            case 2:
                printf( "\nTip: Press 'D' to dock all friendly combat drones currently in the map.\n\n" );
                break;
            case 3:
                printf( "\nTip: When low on resources, recycle drones ('r') or attack and scavenge.\n\n" );
                break;
            case 4:
                printf( "\nTip: Morph a docked combat drone into a Scavenger with 's'.\n\n" );
                break;
            case 5:
                printf( "\nTip: Morph a docked combat drone into a Javelin with 'j'.\n\n" );
                break;
            case 6:
                printf( "\nTip: Morph a docked combat drone into a Miner with 'm'.\n\n" );
                break;
            case 7:
                printf( "\nTip: The javelin's area-of-effect is great for tough or clusters of enemies.\n\n" );
                break;
            case 8:
                printf( "\nTip: Use pause ('p') to pause the action to deploy drones or survey.\n\n" );
                break;
            case 9:
                printf( "\nTip: To get information on an enemy on the map, right-click and then press 'i'.\n\n" );
                break;
        }

    } while ( 0 );

}

// Systems

void behavior_system( void )
{
   for ( int entity = 0 ; entity < MAX_ENTITIES ; entity++ )
   {
      if ( world.mask[ entity ] & COMPONENT_BEHAVIOR )
      {
         (world.behavior[ entity ].behavior)( entity, world.target[ entity ].entity_id );
      }
   }

   return;
}

void heal_system( void )
{
   // Once a drone is selected, 'h' will set the heal marker.
   // Selecting the drone again and pressing 'h' will disable healing.
   // Drones have a heal time, and each delta, one point of HP is restored
   // (while X resources are deducted).

   for ( int entity = 0 ; entity < MAX_ENTITIES ; entity++ )
   {
      if ( world.mask[ entity ] & COMPONENT_HEAL )
      {
         // Does the Borg have resources to heal?
         if ( get_entity_resources( PLAYER_ID ) )
         {
            if ( decrement_heal_speed_cur( entity ) )
            {
               if ( get_entity_health( entity ) < get_entity_max_health( entity ) )
               {
                  // Decrement Borg resources and increment entity health.
                  set_entity_resources( PLAYER_ID, get_entity_resources( PLAYER_ID ) - 1 );
                  set_entity_health( entity, get_entity_health( entity ) + 1 );
                  add_healing_administred( 1 );
               }
               else
               {
                  // Entity is healed fully.
                  stop_entity_healing( entity );
                  add_xp( PLAYER_ID, 2 );
               }
            }
         }
         else
         {
            // Borg is out of resources, disable healing...
            stop_entity_healing( entity );
         }
      }
   }

   return;
}

void morph_system( void )
{
   for ( int entity = 0 ; entity < MAX_ENTITIES ; entity++ )
   {
      if ( world.mask[ entity ] & COMPONENT_MORPH )
      {
         if ( world.mask[ entity ] & COMPONENT_DOCKED )
         {
            if ( --world.morph[ entity ].morph_completion == 0 )
            {
               char ch = world.morph[ entity ].result;

               world.mask[ entity ] &= ~COMPONENT_MORPH;

               // Morph is complete, change the drone.
               if ( ch == 's' )
               {
                  create_edrone_entity( entity, Scavenger, 2, 10, 5, 70 );
               } 
               else if ( ch == 'm' )
               {
                  create_edrone_entity( entity, Miner, 3, 20, 8, 90 );
               }
               else if ( ch == 'j' )
               {
                  create_javelin_entity( entity, 3, 8, 6, 2 );
               }
               else assert(0);
            }
         }
      }
   }

}

void move_system( void )
{
   for ( int entity = 0 ; entity < MAX_ENTITIES ; entity++ )
   {
      if ( world.mask[ entity ] & COMPONENT_MOVEMENT )
      {
         if ( world.target[ entity ].type != TYPE_NO_TARGET )
         {
            world.movement[ entity ].state--;
            if ( world.movement[ entity ].state == 0 )
            {
               int start_row = world.location[ entity ].row;
               int start_col = world.location[ entity ].col;
               int goal_row = 0;
               int goal_col = 0;
               int dRow = 0;
               int dCol = 0;

               world.movement[ entity ].state = world.movement[ entity ].speed;

               // Get the location of the target (coord or entity coord).
               if ( world.target[ entity ].type == TYPE_LOC_TARGET )
               {
                  goal_row = world.target[ entity ].row;
                  goal_col = world.target[ entity ].col;
               }
               else if ( ( world.target[ entity ].type == TYPE_ENTITY_TARGET ) ||
                         ( world.target[ entity ].type == TYPE_ENTITY_DOCK ) )
               {
                  int entity_target = world.target[ entity ].entity_id;
                  goal_row = world.location[ entity_target ].row;
                  goal_col = world.location[ entity_target ].col;
               }

               // Use astar to find the next best step.
               if ( MoveTowardsTarget( goal_row, goal_col, start_row, start_col, &dRow, &dCol) )
               {
                  entity_move( entity, dCol, dRow );
               }
               else
               {
                  // We've reached the target or it's gone.
                  world.target[ entity ].type = TYPE_NO_TARGET;
               }
            }
         }
      }
   }

   return;
}
