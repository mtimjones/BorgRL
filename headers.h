//---------------------------------------------------------------------------
// File: headers.h
//
//   Contains all symbols used in the application.
//
//---------------------------------------------------------------------------

#ifndef __HEADERS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <ncurses.h>
#include <assert.h>
#include "ces.h"

void turn_step( void );

//---------------------------------------------------------------
// Time API
//---------------------------------------------------------------

#define MS_PER_FRAME   40

unsigned long long getTimestamp( void );
void increment_gametime( void );
unsigned int get_gametime( void );

//---------------------------------------------------------------
// Screen API
//---------------------------------------------------------------

void win_startup( );
void win_update( );
void win_refresh( );
char win_wait( );
void win_shutdown( );
chtype  get_user_char( );
void get_mouse_pos( unsigned int *col, unsigned int *row );
void set_context( char *line );
void clear_context( void );
void emit_help( );
void emit_legend( );
void put_char( int col, int row, char cell, unsigned int attr );
int  map_inv_index_to_entity( int index );
int  emit_menu( char *title, char **selections, int num );
void create_explosion( int col, int row, int radius );

//---------------------------------------------------------------
// User Input API
//---------------------------------------------------------------

void handle_user_input( void );
int  get_mouse_col( void );
int  get_mouse_row( void );
void process_command( chtype );

//---------------------------------------------------------------
// Game API
//---------------------------------------------------------------

typedef enum
{
    None        = 0,
    Surrendered = 1,
    Killed      = 2,
    VoidSpace   = 3,
    BossKilled  = 4,

} death_type_t;

void start_game( void );
void end_game( void );
bool get_game_state( void );
void pause_toggle( void );
bool get_pause_state( void );
void set_death_type( death_type_t how );
death_type_t get_death_type( void );


//---------------------------------------------------------------
// Window constants
//---------------------------------------------------------------

#define MAPWIN_COL_START       0
#define MAPWIN_ROW_START       0
#define MAPWIN_COL_SIZE       67
#define MAPWIN_ROW_SIZE       29

#define INVWIN_COL_START      67
#define INVWIN_ROW_START       0
#define INVWIN_COL_SIZE       48
#define INVWIN_ROW_SIZE       22

#define MAX_INVENTORY         ( INVWIN_ROW_SIZE - 3 )

#define STATSWIN_COL_START    67
#define STATSWIN_ROW_START    22
#define STATSWIN_COL_SIZE     48
#define STATSWIN_ROW_SIZE      7

#define ACTIONSWIN_COL_START   0
#define ACTIONSWIN_ROW_START  29
#define ACTIONSWIN_COL_SIZE  115
#define ACTIONSWIN_ROW_SIZE    4

#define CONTEXTWIN_COL_START   0
#define CONTEXTWIN_ROW_START  33
#define CONTEXTWIN_COL_SIZE  115
#define CONTEXTWIN_ROW_SIZE    3

#define LOGWIN_COL_START       0
#define LOGWIN_ROW_START      36
#define LOGWIN_COL_SIZE      115
#define LOGWIN_ROW_SIZE        6

#define INTROWIN_COL_START    24
#define INTROWIN_ROW_START     9
#define INTROWIN_COL_SIZE     54
#define INTROWIN_ROW_SIZE     13

//---------------------------------------------------------------
// Messages API
//---------------------------------------------------------------

#define MAX_MESSAGES 4
#define MAX_MSG_SIZE LOGWIN_COL_SIZE-2

void init_msg_log( void );
void add_message( char *fmt, ... );
char *get_message( int pos );

//---------------------------------------------------------------
// Map API
//---------------------------------------------------------------

#define MAP_SEC_NCOLS ( 9 )
#define MAP_SEC_NROWS ( 3 )
#define MAP_MAX_NCOLS ( MAPWIN_COL_SIZE * MAP_SEC_NCOLS )
#define MAP_MAX_NROWS ( MAPWIN_ROW_SIZE * MAP_SEC_NROWS )

void init_map( void );

#define COLOR_F_ENTITY       1
#define COLOR_F_UNDER_ATTACK 2
#define COLOR_E_ENTITY       3
#define COLOR_E_UNDER_ATTACK 4
#define COLOR_LABEL          5
#define COLOR_BORG           6
#define COLOR_WRECK          7
#define COLOR_PLANET         8
#define COLOR_TRAIL          9
#define COLOR_ACADEMY       10

chtype get_cell( int col, int row );
bool passable( int col, int row );
bool is_cell_uninit( int col, int row );
bool valid_map_location( int col, int row );
bool is_cell_empty( int col, int row );
void set_cell_entity( int col, int row, int entity );
void clear_cell_entity( int col, int row );
void get_exit_location( int *col, int *row );
void set_cell_ephemeral( int col, int row, int delay, chtype contents );
bool find_empty_cell_around_point( int *col, int *row );

void reset_space( void );
void collapse_space( void );

#define MAX_STATES 8

typedef enum
{
   Inactive = 0,
   Active,
} ephstate_t;

typedef enum
{
   type_uninit = 0,
   type_static,
   type_dynamic,
} type_t;

typedef struct
{
   char cell;
} map_static_t;

typedef struct
{
   int cur_delay;
   int delay;
   int state;
   int max_state;
   int cells[ MAX_STATES ];
} map_dynamic_t;

typedef struct
{
   int col;
   int row;
} location_t;

typedef struct
{
   ephstate_t state;
   int cur_delay;
   chtype contents;
} ephemeral_t;

typedef struct
{
   type_t type;

   union {
      map_static_t static_map;
      map_dynamic_t dynamic_map;
   } u;

   location_t location;

   ephemeral_t ephemeral;

   bool visited;
   bool passable;
   // bool visible; // Computed dynamically

   int entity; // Entity here or -1 for none.

} cell_t;

//---------------------------------------------------------------
// CES API
//---------------------------------------------------------------

int get_entity_at( int col, int row );
void cleanup_entities( void );
void entity_move( int entity, int d_col, int d_row );
int get_entity_col( int entity );
int get_entity_row( int entity );
void set_entity_col( int entity, int col );
void set_entity_row( int entity, int row );
void emit_map_context_info( int entity );
void endgame_emit( void );

//---------------------------------------------------------------
// Select API
//---------------------------------------------------------------

void map_button_press( int col, int row, int state );
void inv_button_press( int col, int row, int state );
bool get_inv_selection( int *entity );

//---------------------------------------------------------------
// Player API
//---------------------------------------------------------------

#define PLAYER_COL_START     24
#define PLAYER_ROW_START     43

#define PLAYER_ID             0

//---------------------------------------------------------------
// Levels API
//---------------------------------------------------------------

int  get_level( void );
void inc_level( void );
int  get_max_level( void );

unsigned int get_wreck_count( int level );
unsigned int get_wreck_resources( int level );

unsigned int get_planet_count( int level );
unsigned int get_planet_resources( int level );

unsigned int get_academies( int level );

cdrone_type_t get_cdrone_type( int level, int index );
unsigned int get_combat_drones( int level, int index );
unsigned int get_cd_attack( int level, int index );
unsigned int get_cd_armor( int level, int index );
unsigned int get_cd_hp( int level, int index );
unsigned int get_cd_attack_speed( int level, int index );
unsigned int get_cd_move_speed( int level, int index );

unsigned int get_gas_cloud_rays( int level );
unsigned int get_gas_cloud_ray_len( int level );
unsigned int get_gas_smoothing_param( int level );
unsigned int get_gas_smoothing_filler_param( int level );
unsigned int get_gas_smoothing_iters( int level );

//---------------------------------------------------------------
// Rand API
//---------------------------------------------------------------

#define getSRand( )     ( ( double ) rand( ) / ( double ) RAND_MAX )
#define getRand( x )    ( int ) ( ( x ) * getSRand( ) )
#define seedRand( )     ( srand( time( NULL ) ) )

//---------------------------------------------------------------
// Pathfinding API
//---------------------------------------------------------------

bool MoveTowardsTarget( int start_row, int start_col, int goal_row, int goal_col,
                        int *row, int *col );

#endif // __HEADERS_H__

