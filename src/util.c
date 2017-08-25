#include <util.h>

#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>

#define BASE 10

long try_strtol( char* str )
{
  long  val;
  char* endptr;

  errno = 0;
  
  val = strtol( str, &endptr, BASE );

  /* Check for various possible errors */
  if( ( errno == ERANGE && 
        ( val == LONG_MAX || val == LONG_MIN ) )
      || ( errno != 0 && val == 0 ) ) 
  {
    ( void )fprintf( stderr, "strtol failed, exiting ...\n" );
    exit( EXIT_FAILURE );
  }

  if( endptr == str )
  {
    ( void )fprintf( stderr, "No digits were found, exiting ...\n" );
    exit( EXIT_FAILURE );
  }

  return val;
}

