#include "headers.h"
#include "ces.h"
#include "logapi.h"
#include "stats.h"
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>

bool step = false;

int main( int argc, char *argv[] )
{
   (void)argc;
   (void)argv;

   struct winsize ts;
   ioctl( STDOUT_FILENO, TIOCGWINSZ, &ts );
   if ( ( ts.ws_row < 42 ) || ( ts.ws_col < 115 ) )
   {
      printf( "Please resize your window to 115 cols, 51 rows (currently %d/%d)\n", ts.ws_col, ts.ws_row );
      exit(-1);
   }

   initLog( "log.txt", INFO );

   seedRand( );

   init_stats( );

   init_msg_log( );

   init_map( );

   init_entities( );

   win_startup( );

   receive_random_upgrade( );

   win_update( );

   start_game( );

   // Game loop, runs at 20ms frames.
   while ( get_game_state( ) )
   {
      unsigned long long start = getTimestamp( );

      win_refresh( );

      handle_user_input( );

      if ( ( get_pause_state( ) == false ) || step )
      {
         if ( step ) step = false;

         behavior_system( );

         // target_system( );

         move_system( );

         heal_system( );

         morph_system( );

         // Need to slow this down in the last level.
         collapse_space( );

         increment_gametime( );
      }

      while( getTimestamp( ) < ( start + MS_PER_FRAME ) );
   }

   win_shutdown( );

   closeLog( );

   endgame_emit( );

   return 0;
}

void turn_step( void )
{
   step = true;
}
