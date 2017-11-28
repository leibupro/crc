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

#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


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


int32_t walk_file( uint8_t** buf, ssize_t buf_len, 
                   int* const fd, const char* file )
{
  static uint8_t first_call = 0xFF;
  static uint8_t reset = 0x00;
  struct stat file_stat;
  uint64_t file_size_byte = 0;
  int32_t bytes_read = ( -1 );
  off_t cur_file_pos;

  if( reset )
  {
    first_call = 0xFF;
    reset = 0x00;
    bytes_read = 0;
    return bytes_read;
  }

  if( first_call )
  {
    first_call = 0x00;
    *fd = ( -1 );

    if( !( *buf = ( uint8_t* )malloc( buf_len * sizeof( uint8_t ) ) ) )
    {
      ( void )fprintf( stderr, "Failed to allocate workspace memory.\n" );
      exit( EXIT_FAILURE );
    }

    if( ( *fd = open( file, O_RDONLY ) ) == ( -1 ) )
    {
      ( void )fprintf( stderr, "%s\n", strerror( errno ) );
      free( *buf );
      exit( EXIT_FAILURE );
    }

    if( stat( file, &file_stat ) )
    {
      ( void )fprintf( stderr, "%s\n", strerror( errno ) );
      free( *buf );
      exit( EXIT_FAILURE );
    }
    file_size_byte = file_stat.st_size;
    ( void )fprintf( stdout, "\nThe input file has a size of %ld bytes.\n", 
                             file_size_byte );
  }

  bytes_read = ( int32_t )read( *fd, ( void* )( *buf ), buf_len );

  cur_file_pos = lseek( *fd, 0, SEEK_CUR );
  if( cur_file_pos == lseek( *fd, 0, SEEK_END ) )
  {
    /* we are at the end of the file and can free resources */
    ( void )close( *fd );
    *fd = ( -1 );
    free( *buf );
    *buf = NULL;
    reset = 0xFF;
  }
  else
  {
    ( void )lseek( *fd, cur_file_pos, SEEK_SET );
  }

  return bytes_read;
}

