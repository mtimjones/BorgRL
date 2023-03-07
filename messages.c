#include "headers.h"

static int write_ptr;

char messages[ MAX_MESSAGES ][ MAX_MSG_SIZE ];

void init_msg_log( void )
{
   write_ptr = 0;

   bzero( messages, sizeof( messages ) );

   return;
}

void add_message( char *fmt, ... )
{
   va_list args;
   char line[LOGWIN_COL_SIZE];

   va_start( args, fmt );
   vsprintf( line, fmt, args );
   strcpy( messages[ write_ptr ], line );

   int len = strlen( messages[ write_ptr ] );
   while ( len < LOGWIN_COL_SIZE-3 )
   {
      messages[ write_ptr ][ len++ ] = ' ';
   }
   messages[ write_ptr ][ len ] = 0;

   if ( ++write_ptr >= MAX_MESSAGES )
   {
      write_ptr = 0;
   }

   return;
}

char *get_message( int pos )
{
   int read_ptr = write_ptr + pos;

   if ( read_ptr >= MAX_MESSAGES )
   {
      read_ptr -= MAX_MESSAGES;
   }

   return &messages[ read_ptr ][ 0 ];
}

