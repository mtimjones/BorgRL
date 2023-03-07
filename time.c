#include "headers.h"

unsigned int GameTime = 0;

unsigned long long getTimestamp( void )
{
   struct timeval tv;
   unsigned long long tl;

   gettimeofday( &tv, NULL );

   tl = ( tv.tv_sec * 1000000 ) + tv.tv_usec;

   // Convert to milliseconds
   tl = tl / 1000;

   return tl;
}

void increment_gametime( void )
{
   GameTime++;
}

unsigned int get_gametime( void )
{
   return GameTime;
}

