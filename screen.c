#include "headers.h"
#include "ces.h"
#include "logapi.h"
#include <math.h>

WINDOW *mapwin, *invwin, *statswin, *actionswin, *contextwin, *logwin;

#define win_redisplay( win ) { touchwin( win ); wrefresh( win ); }

void render_map( void );
void render_inv( void );

static char context[ CONTEXTWIN_COL_SIZE+1 ];

// inv_map[ inv_index ] = entity
static int inv_map[ MAX_INVENTORY ];

static void init_colors( void )
{
   if ( !has_colors( ) ) exit(-1);

   start_color( );

   init_pair( COLOR_F_ENTITY,       COLOR_WHITE,   COLOR_BLACK );
   init_pair( COLOR_F_UNDER_ATTACK, COLOR_WHITE,   COLOR_RED );
   init_pair( COLOR_E_ENTITY,       COLOR_RED,     COLOR_BLACK );
   init_pair( COLOR_E_UNDER_ATTACK, COLOR_BLACK,   COLOR_RED );
   init_pair( COLOR_LABEL,          COLOR_CYAN,    COLOR_BLACK );
   init_pair( COLOR_BORG,           COLOR_WHITE,   COLOR_BLACK );
   init_pair( COLOR_WRECK,          COLOR_YELLOW,  COLOR_BLACK );
   init_pair( COLOR_PLANET,         COLOR_BLUE,    COLOR_BLACK );
   init_pair( COLOR_TRAIL,          COLOR_CYAN,    COLOR_BLACK );
   init_pair( COLOR_ACADEMY,        COLOR_MAGENTA, COLOR_BLACK );

   return;
}

void win_startup( )
{
   initscr( );
   clear( );
   noecho( );
   curs_set( 0 );
   nonl( );
   cbreak( );
   keypad( stdscr, TRUE );
   init_colors( );

   memset( inv_map, 0, sizeof(inv_map) );

   mouseinterval(0);
   printf("\033[?1003h\n");

   mousemask( ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL );

   return;
}

void win_update( )
{
   mapwin = newwin( MAPWIN_ROW_SIZE, MAPWIN_COL_SIZE, 
                     MAPWIN_ROW_START, MAPWIN_COL_START );
   box( mapwin, 0, 0 );
   wattron( mapwin, A_BOLD | COLOR_PAIR(COLOR_LABEL) );
   mvwprintw( mapwin, 0, 2, " Map " );
   wattroff( mapwin, A_BOLD | COLOR_PAIR(COLOR_LABEL) );

   invwin = newwin( INVWIN_ROW_SIZE, INVWIN_COL_SIZE,
                       INVWIN_ROW_START, INVWIN_COL_START );
   box( invwin, 0, 0 );
   wattron( invwin, A_BOLD | COLOR_PAIR(COLOR_LABEL) );
   mvwprintw( invwin, 0, 2, " Inventory (dock) " );
   wattroff( invwin, A_BOLD | COLOR_PAIR(COLOR_LABEL) );
   mvwprintw( invwin, 1, 2, "Object.... State..... Lvl HP Max Amr Att Rsc" );

   statswin = newwin( STATSWIN_ROW_SIZE, STATSWIN_COL_SIZE,
                       STATSWIN_ROW_START, STATSWIN_COL_START );
   box( statswin, 0, 0 );
   wattron( statswin, A_BOLD | COLOR_PAIR(COLOR_LABEL) );
   mvwprintw( statswin, 0, 2, " Stats " );
   wattroff( statswin, A_BOLD | COLOR_PAIR(COLOR_LABEL) );
   mvwprintw( statswin, 1, 2, "Attack: *** " );
   mvwprintw( statswin, 2, 2, "Armor : *** " );
   mvwprintw( statswin, 1, 17, "Mining    : *** " );
   mvwprintw( statswin, 2, 17, "Scavenging: *** " );
   mvwprintw( statswin, 5, 17, "Resources : %3d ", get_entity_resources( PLAYER_ID ) );

   actionswin = newwin( ACTIONSWIN_ROW_SIZE, ACTIONSWIN_COL_SIZE, 
                         ACTIONSWIN_ROW_START, ACTIONSWIN_COL_START );
   box( actionswin, 0, 0 );
   wattron( actionswin, A_BOLD | COLOR_PAIR(COLOR_LABEL) );
   mvwprintw( actionswin, 0, 2, " Actions " );
   wattroff( actionswin, A_BOLD | COLOR_PAIR(COLOR_LABEL) );
   mvwprintw( actionswin, 1, 3, "p - pause/unpause | Select docked drone and [ h - heal / r - recycle / select object in map ]." );
   mvwprintw( actionswin, 2, 3, "? - help          | Select enemy and [ a - assimilate ]. Select undocked drone and [ d - dock ]." );

   contextwin = newwin( CONTEXTWIN_ROW_SIZE, CONTEXTWIN_COL_SIZE, 
                         CONTEXTWIN_ROW_START, CONTEXTWIN_COL_START );
   box( contextwin, 0, 0 );
   wattron( contextwin, A_BOLD | COLOR_PAIR(COLOR_LABEL) );
   mvwprintw( contextwin, 0, 2, " Context " );
   wattroff( contextwin, A_BOLD | COLOR_PAIR(COLOR_LABEL) );

   logwin = newwin( LOGWIN_ROW_SIZE, LOGWIN_COL_SIZE,
                     LOGWIN_ROW_START, LOGWIN_COL_START );
   box( logwin, 0, 0 );
   wattron( logwin, A_BOLD | COLOR_PAIR(COLOR_LABEL) );
   mvwprintw( logwin, 0, 2, " Log " );
   wattroff( logwin, A_BOLD | COLOR_PAIR(COLOR_LABEL) );
   add_message( "Press '?' for help, 'l' for the legend." );

   refresh( );

   WINDOW *introwin = newwin( INTROWIN_ROW_SIZE, INTROWIN_COL_SIZE,
                               INTROWIN_ROW_START, INTROWIN_COL_START );
   box( introwin, 0, 0 );
   wattron( introwin, A_BOLD | COLOR_PAIR(COLOR_LABEL) );
   mvwprintw( introwin, 0, 2, " Introduction " );
   wattroff( introwin, A_BOLD | COLOR_PAIR(COLOR_LABEL) );
   mvwprintw( introwin, 2, 2, "Welcome to BorgRL, a submission to the 2023 7DRL." );
   mvwprintw( introwin, 3, 2, "As the Borg, you'll assimilate enemies or attack" );
   mvwprintw( introwin, 4, 2, "them as well as scavenge wrecks and mine planets" );
   mvwprintw( introwin, 5, 2, "for resources (to upgrade your drones and their" );
   mvwprintw( introwin, 6, 2, "abilities).  Manage your drones and resources" );
   mvwprintw( introwin, 7, 2, "in order to reach the end and defeat the boss." );
   mvwprintw( introwin, 8, 2, "Space is collapsing behind you..." );
   mvwprintw( introwin, 10, 2, "Press the spacebar to begin the game." );

   wrefresh( mapwin );
   wrefresh( invwin );
   wrefresh( statswin );
   wrefresh( actionswin );
   wrefresh( contextwin );
   wrefresh( logwin );
   wrefresh( introwin );

   win_wait( );

   delwin( introwin );
   wrefresh( stdscr );

   // Turn on nodelay here since the intro screen is done.
   nodelay( stdscr, TRUE );
}

void win_refresh( void )
{
   render_map( );
   win_redisplay( mapwin );

   render_inv( );
   win_redisplay( invwin );

   if ( get_pause_state( ) )
   {
      wattron( statswin, A_STANDOUT | A_BLINK );
      mvwprintw( statswin, 3, 2, "Game paused." );
      wattroff( statswin, A_STANDOUT | A_BLINK );
   }
   else
   {
      mvwprintw( statswin, 3, 2, "            " );
   }

   mvwprintw( statswin, 3, 17, "Mouse  %3d, %3d", get_mouse_col( ), get_mouse_row( ) );
   mvwprintw( statswin, 4, 17, "Player %3d, %3d", get_entity_col( PLAYER_ID ), get_entity_row( PLAYER_ID ) );

   mvwprintw( statswin, 4, 2, "Sector : %3d ", get_level( ) + 1 );
   mvwprintw( statswin, 5, 2, "Time  : %4d ", ( get_gametime( ) / 25 ) );
   mvwprintw( statswin, 5, 17, "Resources : %3d ", get_entity_resources( PLAYER_ID ) );

   wrefresh( statswin );
//   win_redisplay( statswin );

   mvwprintw( contextwin, 1, 2, "%s", context );
   wrefresh( contextwin );

   for ( int i = 0 ; i < MAX_MESSAGES ; i++ )
   {
      mvwprintw( logwin, (i+1), 2, "%s", get_message( i ) );
   }
   wrefresh( logwin );

   wrefresh( stdscr );
}

char win_wait( )
{
   chtype ch;
   while ( ( ch = wgetch( stdscr ) ) != ' ' );
   return ch;
}

void win_shutdown( )
{
   // Disable mouse events
   printf("\033[?1003l\n");

   delwin( logwin );
   delwin( contextwin );
   delwin( actionswin );
   delwin( statswin );
   delwin( invwin );
   delwin( mapwin );

   endwin();

   return;
}

void set_context( char *line )
{
   strncpy( context, line, CONTEXTWIN_COL_SIZE-4 );
   int len = strlen( context );
   while ( len < CONTEXTWIN_COL_SIZE-4 )
   {
      context[len++] = ' ';
   }
   context[len] = 0;
}

void clear_context( void )
{
   char line[CONTEXTWIN_COL_SIZE];
   memset( line, ' ', sizeof( line ) );
   line[CONTEXTWIN_COL_SIZE-2] = 0;
   set_context( line );
}

void emit_help( void )
{
   WINDOW *helpwin = newwin( 14, 70, 8, 15 );
   box( helpwin, 0, 0 );
   wattron( helpwin, A_BOLD | COLOR_PAIR(COLOR_LABEL) );
   mvwprintw( helpwin, 0, 2, " Help " );
   wattroff( helpwin, A_BOLD | COLOR_PAIR(COLOR_LABEL) );
   mvwprintw( helpwin, 2, 2, "Make your way through each sector, escaping through the gate." );
   mvwprintw( helpwin, 3, 2, "Fight, mine, scavange, heal, recycle, and upgrade your drones." );
   mvwprintw( helpwin, 4, 2, "Move with right mouse. Select a drone from the map or inventory" );
   mvwprintw( helpwin, 5, 2, "window (left-click) and a target in the map window to begin." );
   mvwprintw( helpwin, 6, 2, "Select an enemy and 'i' for information or 'a' to assimilate." );
   mvwprintw( helpwin, 7, 2, "Select a docked drone and 'h' to heal it or 'r' to recycle it" );
   mvwprintw( helpwin, 8, 2, "for resources.  Select a docked combat drone to morph into 's'" );
   mvwprintw( helpwin, 9, 2, "(Scav), 'm' (Miner), or 'j' (Javelin).  Surrender with 'X'." );
   mvwprintw( helpwin, 11, 2, "                 Press space to continue." );
   nodelay( stdscr, FALSE );
   wrefresh( helpwin );
   win_wait( );
   nodelay( stdscr, TRUE );
   wrefresh( stdscr );
}

void emit_legend( void )
{
   WINDOW *legendwin = newwin( 10, 67, 10, 15 );
   box( legendwin, 0, 0 );
   wattron( legendwin, A_BOLD | COLOR_PAIR(COLOR_LABEL) );
   mvwprintw( legendwin, 0, 2, " Legend " );
   wattroff( legendwin, A_BOLD | COLOR_PAIR(COLOR_LABEL) );
   mvwprintw( legendwin, 2, 2, "@ - The Borg    | # - Gas Cloud  | A - Academy    | R - Raven");
   mvwprintw( legendwin, 3, 2, "* - Marker      | %% - Wreck      | S - Scavenger  | H - Hunter" );
   mvwprintw( legendwin, 4, 2, ". - Trail       | P - Planet     | M - Miner      | K - Kestral" );
   mvwprintw( legendwin, 5, 2, "~ - Void Space  | > - Star Gate  | N - Nova       | T - Tank" );
   mvwprintw( legendwin, 7, 23, "Press space to continue." );
   nodelay( stdscr, FALSE );
   wrefresh( legendwin );
   win_wait( );
   nodelay( stdscr, TRUE );
   wrefresh( stdscr );
}


int emit_menu( char *title, char *selections[], int num )
{
   char cont[50];
   int max_length = 0;
   int result;

   sprintf( cont, "%d resources.  Press space to exit.", UPGRADE_COST );

   for ( int i = 0 ; i < num ; i++ )
   {
      int length = strlen( selections[ i ] );
      if ( length > max_length ) max_length = length;
   }

   max_length += 8;
   if ( max_length < (int)strlen( cont ) ) max_length = (int)strlen( cont ) + 4;

   WINDOW *menuwin = newwin( num + 2 + 3, max_length, 10, 15 );
   box( menuwin, 0, 0 );
   wattron( menuwin, A_BOLD | COLOR_PAIR( COLOR_LABEL ) );
   mvwprintw( menuwin, 0, 2, " %s ", title );
   wattroff( menuwin, A_BOLD | COLOR_PAIR( COLOR_LABEL ) );
   for ( int i = 0 ; i < num ; i++ )
   {
      mvwprintw( menuwin, 2+i, 2, "%1d) %s", i, selections[i] );
   }

   mvwprintw( menuwin, num+3, (max_length-strlen(cont))/2, "%s", cont );

   nodelay( stdscr, FALSE );
   wrefresh( menuwin );

   while ( 1 )
   {
      char ch = (char)get_user_char( );
      if ( ch == ' ' )
      {
          result = -1;
          break;
      }
      else
      {
         if ( ( ch >= '0' ) && ( ch < ('0'+num) ) ) 
         {
            result = ch - '0';
            break;
         }
      }
   }

   nodelay( stdscr, TRUE );
   wrefresh( stdscr );

   return result;
}

int get_user_char( void )
{
   int c = wgetch( stdscr );

   if ( c != ERR ) return c;

   return 0;
}

void put_char( int col, int row, char cell, unsigned int attr )
{
   mvwaddch( mapwin, row+1, col+1, cell | attr );
}

void render_map( void )
{
   const int map_col_midpoint = ( ( MAPWIN_COL_SIZE-2 ) >> 1 );
   const int map_row_midpoint = ( ( MAPWIN_ROW_SIZE-2 ) >> 1 );
   int prow = get_entity_row( PLAYER_ID );
   int pcol = get_entity_col( PLAYER_ID );
   int row, col;

   for ( int r = 0 ; r < ( MAPWIN_ROW_SIZE-2 ) ; r++ )
   {
      row = prow - map_row_midpoint + r;

      for ( int c = 0 ; c < ( MAPWIN_COL_SIZE-2 ) ; c++ )
      {
         col = pcol - map_col_midpoint + c;
         mvwaddch( mapwin, r+1, c+1, get_cell( col, row ) );
      }

   }

   return;
}

void render_inv( void )
{
   char object[11];
   char state [11];
   int  level, hp, max_hp, armor, attack, resources;
   int  index = 0;

   for ( int i = 0 ; i < MAX_ENTITIES ; i++ )
   {
      bool result = get_player_inv( i, object, state, &level, &hp, &max_hp, &armor, &attack, &resources );

      if ( result ) 
      {
         int  selected_entity;
         bool result = get_inv_selection( &selected_entity );

         inv_map[ index ] = i;

         if ( is_entity_morphing( i ) ) wattron( invwin, A_DIM );
         else if ( result && ( inv_map[ index ] == selected_entity ) ) wattron( invwin, A_STANDOUT );
         mvwprintw( invwin, 2+index, 2, "%10s %10s %3d %2d %3d %3d %3d %3d", 
                     object, state, level, hp, max_hp, armor, attack, resources );
         wattroff( invwin, A_STANDOUT | A_DIM );

         if ( ++index >= MAX_INVENTORY) break;
      }
   }

   for ( ; index < MAX_INVENTORY ; index++ )
   {
      inv_map[ index ] = -1;
      mvwprintw( invwin, 2+index, 2, "                                             " );
   }

   return;
}

int map_inv_index_to_entity( int index )
{
   return inv_map[ index ];
}

void create_explosion( int col, int row, int radius )
{
   for ( int i = 1 ; i < radius ; i++ )
   {
      int count = ( radius * radius ) * i;

      while ( count-- )
      {
         double a = getSRand( ) * 2 * 3.1415926;
         double r = radius * sqrt( getSRand( ) );
         double x = r * cos( a ) + ( double )col;
         double y = ( ( r * sin( a ) ) * 0.7 ) + ( double )row + 1;

         set_cell_ephemeral( (int)x, (int)y, (int)(r*5), '*' );
      }
   }
}
