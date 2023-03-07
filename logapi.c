// logging api implementation
//

#include "logapi.h"

static unsigned int loglevel = INFO;
static FILE *fptr;

void initLog( char *filename, LogLevel_t level )
{
   loglevel = level;

   fptr = fopen( filename, "w" );
}

void logger( LogLevel_t level, char *fmt, ... )
{
   va_list args;
   if ( level <= loglevel )
   {
      va_start( args, fmt );
      vfprintf( fptr, fmt, args );
      va_end( args );
      fflush( fptr );
   }

   return;
}

void closeLog( void )
{
   fclose( fptr );

   return;
}
