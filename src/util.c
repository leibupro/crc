/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <pl@vqe.ch> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   P. Leibundgut
 * ----------------------------------------------------------------------------
 *
 * File:      util.c
 * 
 *
 * Purpose:   Helper module holds some generic
 *            functions to do this and that.
 *
 * 
 * Remarks:   -
 *
 *
 * Date:      09/2017 
 * 
 */

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

