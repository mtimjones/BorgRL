// logapi.h
//
// logging api
//

#ifndef __LOGAPI_H
#define __LOGAPI_H

#include <stdio.h>
#include <stdarg.h>

typedef enum
{
   FATAL,
   WARNING,
   INFO
} LogLevel_t;

void initLog( char *filename, LogLevel_t loglevel );

void logger( LogLevel_t level, char *fmt, ... );

void closeLog( void );

#endif // __LOGAPI_H
