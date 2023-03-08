// ces.h
#include <ncurses.h>
#include <stdbool.h>

#ifndef __COMPONENT_H__
#define __COMPONENT_H__

// Resources and Time constants

#define MORPH_TIME           360
#define MAX_HEAL_SPEED        60
#define PLAYER_MOVE_SPEED      4
#define SCAVENGER_MOVE_SPEED  10
#define MINER_MOVE_SPEED      18
#define SCAV_SPEED            70
#define MINE_SPEED            90

#define BOSS_DRONE_COUNT       7
#define BOSS_DRONE_WAIT_TIME 160

#define ASSIMILATE_COST         (  7 + get_level( ) * 4 )
#define UPGRADE_COST            ( 15 )
#define INITIAL_PLAYER_RESOURCES  25
#define RANDOM_RES_UPGRADE      ( 10 + getRand( 5 ) )
#define DRONE_KILL_XP           (  3 + get_level( ) )

#define MAX_DRONE_TYPES            5

typedef struct component_location
{
    double col;
    double row;
} component_location_t;

typedef struct component_health
{
    int value;
    int max_health;
} component_health_t;

typedef struct component_attack
{
    int value;
    int armor;
    int attack_speed;
    int attack_speed_state;
} component_attack_t;

typedef enum
{
   TYPE_NO_TARGET        = 0,
   TYPE_LOC_TARGET       = 1,  // Used to move an entity to a specific location.
   TYPE_ENTITY_TARGET    = 2,  // Used to move an entity to another entity.
   TYPE_ENTITY_DOCK      = 3,  // Move an entity to dock with the target.
   TYPE_ENTITY_ORBIT     = 4,  // Move an entity to orbit around another entity.
   TYPE_ENTITY_PROXIMITY = 5,  // Used to move an entity within some distance to another.
} cmovement_type_t;

typedef struct component_movement
{
    int speed;
    int state;
} component_movement_t;

#define BASE_XP      ((float)16)
#define FACTOR       ((float)1.2)

typedef struct component_xp
{
    int value;
    int level;
} component_xp_t;

typedef struct component_heal
{
    int heal_speed_cur;
} component_heal_t;

typedef struct component_resources
{
    int value;
} component_resources_t;

typedef struct component_target
{
    cmovement_type_t type;
    int distance;
    int entity_id;
    int cached_entity_id;
    int col;
    int row;
} component_target_t;

typedef struct component_render
{
   char cell;
   int  attr;
   int  attr_alternate;
   int  attr_dec;
} component_render_t;

typedef enum 
{
   Scavenger = 0,
   Miner = 1,
} edrone_type_t;

typedef enum
{
   Nova    = 0,
   Raven   = 1,
   Hunter  = 2,
   Kestral = 3,
   Tank    = 4,
   Javelin = 5,
} cdrone_type_t;

// Extraction drone component
typedef struct component_edrone
{
   edrone_type_t type;
   int max_resources;
   int extract_speed;
   int extract_state;
} component_edrone_t;

typedef struct component_cdrone
{
   cdrone_type_t type;
} component_cdrone_t;

typedef enum
{
   Dormant = 0,

   // Scavenging / Mining
   TravelingToResources,
   ScavengingResources,
   ReturningToBorg,
   RedockingWithBorg,

   // Academy
   TravelingToAcademy,
   ReturningFromAcademy,

   // Enemy Combat
   MovingTowardsTarget,
   AttackingTarget,

   // Boss Behavior
   Awake,
   Done,

} behavior_state_t;

typedef void (*behavior_t)( int entity, int target_entity );

typedef struct component_behavior
{
   behavior_state_t state;
   behavior_t       behavior;
} component_behavior_t;

typedef void (*on_verb_callback)( int dest_entity, int source_entity );

typedef struct entity_callbacks
{
   on_verb_callback on_attack; // source_entity attacks dest_entity.
   on_verb_callback on_death;  // dest_entity killed by source_entity.
} entity_callbacks_t;

typedef struct component_morph
{
   char result;
   int  morph_completion;
} component_morph_t;

#define MAX_STRING    10
#define MAX_STR_STATE  5

typedef struct 
{
   char name_str[ MAX_STRING+1 ];
} entity_name_t;

typedef struct
{
   char state_str[ MAX_STRING+1 ];
} entity_state_t;

typedef enum
{
    COMPONENT_NONE       = 0,

    COMPONENT_LOCATION   = 1 <<  0,
    COMPONENT_HEALTH     = 1 <<  1,
    COMPONENT_RESOURCES  = 1 <<  2,
    COMPONENT_RENDER     = 1 <<  3,

    COMPONENT_MOVEMENT   = 1 <<  4, // Movement system
    COMPONENT_HEAL       = 1 <<  5, // Healing system (in Borg).
    COMPONENT_MORPH      = 1 <<  6, // Morph system (in Borg).
    COMPONENT_TARGET     = 1 <<  7, // Targeting system.
    COMPONENT_BEHAVIOR   = 1 <<  8, // Behavior system (state machine for the entity).

    COMPONENT_PLAYER     = 1 <<  9, // Marker
    COMPONENT_FRIENDLY   = 1 << 10, // Marker
    COMPONENT_ENEMY      = 1 << 11, // Marker

    COMPONENT_DOCKED     = 1 << 12, // Marker
    COMPONENT_CDRONE     = 1 << 13, // Marker / Combat Drone
    COMPONENT_EDRONE     = 1 << 14, // Marker / Extractor Drone (Scavenger, Miner)

    COMPONENT_WRECK      = 1 << 15, // Marker
    COMPONENT_PLANET     = 1 << 16, // Marker
    COMPONENT_ACADEMY    = 1 << 17, // Marker

} Component;

#define NUMBER_OF_BORG        1
#define NUMBER_OF_WRECKS     10
#define NUMBER_OF_DRONES     30
#define NUMBER_OF_PLANETS     6
#define NUMBER_OF_ACADEMIES   3
#define NUMBER_OF_STARBASES   1

#define NO_ENTITY            -1

#define MAX_ENTITIES \
            ( NUMBER_OF_BORG + NUMBER_OF_WRECKS + NUMBER_OF_DRONES + \
              NUMBER_OF_PLANETS + NUMBER_OF_ACADEMIES + NUMBER_OF_STARBASES )

typedef struct World
{
    unsigned int mask[ MAX_ENTITIES ];

    unsigned int id  [ MAX_ENTITIES ];

    component_location_t   location  [ MAX_ENTITIES ];
    component_health_t     health    [ MAX_ENTITIES ];
    component_attack_t     attack    [ MAX_ENTITIES ];
    component_movement_t   movement  [ MAX_ENTITIES ];
    component_xp_t         xp        [ MAX_ENTITIES ];
    component_resources_t  resources [ MAX_ENTITIES ];
    component_target_t     target    [ MAX_ENTITIES ];
    component_render_t     render    [ MAX_ENTITIES ];
    component_edrone_t     edrone    [ MAX_ENTITIES ];
    component_cdrone_t     cdrone    [ MAX_ENTITIES ];
    entity_callbacks_t     callbacks [ MAX_ENTITIES ];
    component_behavior_t   behavior  [ MAX_ENTITIES ];
    entity_name_t          ename     [ MAX_ENTITIES ];
    entity_state_t         estate    [ MAX_ENTITIES ];
    component_heal_t       heal      [ MAX_ENTITIES ];
    component_morph_t      morph     [ MAX_ENTITIES ];

} World;

void  init_entities( );

bool  get_player_inv( int entity, char *object, char *state, int *level, int *hp, int *max_hp, int *armor, int *attack, int *resources );

void  set_entity_behavior_state( int entity, behavior_state_t state );
behavior_state_t get_entity_behavior_state( int entity );
void  start_entity_behavior( int entity );
void  stop_entity_behavior( int entity );

bool  get_entity_render( int entity, chtype *ch );
int   get_entity_max_health( int entity );
int   get_entity_health( int entity );
void  set_entity_health( int entity, int value );
void  set_entity_render_alternate_attr( int entity, chtype ch, int count );

bool  is_entity_docked( int entity );
bool  is_entity_friendly( int entity );
bool  is_entity_enemy( int entity );
bool  is_entity_morphing( int entity );

void  create_wreck_entity( int col, int row, int resources );
void  demote_wreck_entity( int entity );

void create_cedrone_entity( cdrone_type_t type, int col, int row,
                            int resources, int health, int speed, int attack, int attack_speed,
                            int armor, int xp );

void  javelin_explosion( int entity );

void  destroy_entity( int entity );

void  set_entity_resources( int entity, int value );
int   get_entity_resources( int entity );

int   get_entity_attack( int entity );
int   get_entity_armor( int entity );

int   get_entity_level( int entity );

void  set_entity_docked( int entity );
void  reset_entity_docked( int entity );

void  start_entity_healing( int entity );
void  stop_entity_healing( int entity );

void  set_morph_data( int entity, int delay, char type );

void  set_target_loc( int entity, cmovement_type_t mtype, int col, int row );
void  set_target_entity( int entity, cmovement_type_t mtype, int target_entity );
void  set_target_dock( int entity, cmovement_type_t mtype, int target_entity );

void  set_entity_target( int entity, int target_entity );
void  clear_entity_target( int entity );
void  set_cached_entity_target( int entity, int target_entity );
int   get_cached_entity_target( int entity );

void  set_entity_state_str( int entity, char *state );
char* get_entity_state_str( int entity );

void  set_entity_name_str( int entity, char *name );
char* get_entity_name_str( int entity );
char* get_entity_name_str_trim( int entity );

bool  decrement_heal_speed_cur( int entity );

int  get_edrone_max_resources( int entity );
int  get_edrone_speed( int entity );
int  get_edrone_state( int entity );
void set_edrone_state( int entity, int value );
char* get_edrone_action_str( int entity );

bool is_friendly( int entity );
bool is_entity_cdrone( int entity );
bool is_enemy( int entity );
bool is_wreck( int entity );
bool is_planet( int entity );
bool is_academy( int entity );
bool is_target_compatible( int entity, int target );
bool is_friendly_combat_drone( int entity );
bool is_entity_miner( int entity );
bool is_entity_scavenger( int entity );

void attack( int entity, int target_entity );
void assimilate( int entity );
float get_assimilate_probability( int attacker, int attackee );

void add_xp( int entity, int xpoints );

void  move_system( void );
void  heal_system( void );
void  behavior_system( void );
void  morph_system( void );

#define MAX_RADAR_DISTANCE    14

int   find_close_entity( int entity );
int   entity_distance( int entity, int target );

void receive_random_upgrade( void );
void get_random_academy_upgrade( int entity, bool dec_resources );
void academy_edrone_upgrade( int entity );
void academy_cdrone_upgrade( int entity );

#endif // __COMPONENT_H__

