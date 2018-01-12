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


inline void reflect_bits_8( uint8_t* field, uint32_t n )
{
  uint32_t i;
  uint8_t b;

  for( i = 0; i < n; i++ )
  {
    b = *( field + i );
    b = ( ( b & 0x80U ) >> 7U ) | ( ( b & 0x01U ) << 7U ) |
        ( ( b & 0x40U ) >> 5U ) | ( ( b & 0x02U ) << 5U ) |
        ( ( b & 0x20U ) >> 3U ) | ( ( b & 0x04U ) << 3U ) |
        ( ( b & 0x10U ) >> 1U ) | ( ( b & 0x08U ) << 1U ) ;
    *( field + i ) = b;
  }
}


inline void reflect_bits_16( uint16_t* field, uint32_t n )
{
  uint32_t i;
  uint16_t db;

  for( i = 0; i < n; i++ )
  {
    db = *( field + i );
    db = ( ( db & 0x8000U ) >> 15U ) | ( ( db & 0x0001U ) << 15U ) |
         ( ( db & 0x4000U ) >> 13U ) | ( ( db & 0x0002U ) << 13U ) |
         ( ( db & 0x2000U ) >> 11U ) | ( ( db & 0x0004U ) << 11U ) |
         ( ( db & 0x1000U ) >>  9U ) | ( ( db & 0x0008U ) <<  9U ) |
         ( ( db & 0x0800U ) >>  7U ) | ( ( db & 0x0010U ) <<  7U ) |
         ( ( db & 0x0400U ) >>  5U ) | ( ( db & 0x0020U ) <<  5U ) |
         ( ( db & 0x0200U ) >>  3U ) | ( ( db & 0x0040U ) <<  3U ) |
         ( ( db & 0x0100U ) >>  1U ) | ( ( db & 0x0080U ) <<  1U ) ;
    *( field + i ) = db;
  }
}


inline void reflect_bits_32( uint32_t* field, uint32_t n )
{
  uint32_t i;
  uint32_t w;

  for( i = 0; i < n; i++ )
  {
    w = *( field + i );
    w = ( ( w & 0x80000000U ) >> 31U ) | ( ( w & 0x00000001U ) << 31U ) |
        ( ( w & 0x40000000U ) >> 29U ) | ( ( w & 0x00000002U ) << 29U ) |
        ( ( w & 0x20000000U ) >> 27U ) | ( ( w & 0x00000004U ) << 27U ) |
        ( ( w & 0x10000000U ) >> 25U ) | ( ( w & 0x00000008U ) << 25U ) |
        ( ( w & 0x08000000U ) >> 23U ) | ( ( w & 0x00000010U ) << 23U ) |
        ( ( w & 0x04000000U ) >> 21U ) | ( ( w & 0x00000020U ) << 21U ) |
        ( ( w & 0x02000000U ) >> 19U ) | ( ( w & 0x00000040U ) << 19U ) |
        ( ( w & 0x01000000U ) >> 17U ) | ( ( w & 0x00000080U ) << 17U ) |
        ( ( w & 0x00800000U ) >> 15U ) | ( ( w & 0x00000100U ) << 15U ) |
        ( ( w & 0x00400000U ) >> 13U ) | ( ( w & 0x00000200U ) << 13U ) |
        ( ( w & 0x00200000U ) >> 11U ) | ( ( w & 0x00000400U ) << 11U ) |
        ( ( w & 0x00100000U ) >>  9U ) | ( ( w & 0x00000800U ) <<  9U ) |
        ( ( w & 0x00080000U ) >>  7U ) | ( ( w & 0x00001000U ) <<  7U ) |
        ( ( w & 0x00040000U ) >>  5U ) | ( ( w & 0x00002000U ) <<  5U ) |
        ( ( w & 0x00020000U ) >>  3U ) | ( ( w & 0x00004000U ) <<  3U ) |
        ( ( w & 0x00010000U ) >>  1U ) | ( ( w & 0x00008000U ) <<  1U ) ;
    *( field + i ) = w;
  }
}


inline void reflect_bits_64( uint64_t* field, uint32_t n )
{
  uint32_t i;
  uint64_t dw;

  for( i = 0; i < n; i++ )
  {
    dw = *( field + i );
    dw = ( ( dw & 0x8000000000000000UL ) >> 63U ) | ( ( dw & 0x0000000000000001UL ) << 63U ) |
         ( ( dw & 0x4000000000000000UL ) >> 61U ) | ( ( dw & 0x0000000000000002UL ) << 61U ) |
         ( ( dw & 0x2000000000000000UL ) >> 59U ) | ( ( dw & 0x0000000000000004UL ) << 59U ) |
         ( ( dw & 0x1000000000000000UL ) >> 57U ) | ( ( dw & 0x0000000000000008UL ) << 57U ) |
         ( ( dw & 0x0800000000000000UL ) >> 55U ) | ( ( dw & 0x0000000000000010UL ) << 55U ) |
         ( ( dw & 0x0400000000000000UL ) >> 53U ) | ( ( dw & 0x0000000000000020UL ) << 53U ) |
         ( ( dw & 0x0200000000000000UL ) >> 51U ) | ( ( dw & 0x0000000000000040UL ) << 51U ) |
         ( ( dw & 0x0100000000000000UL ) >> 49U ) | ( ( dw & 0x0000000000000080UL ) << 49U ) |
         ( ( dw & 0x0080000000000000UL ) >> 47U ) | ( ( dw & 0x0000000000000100UL ) << 47U ) |
         ( ( dw & 0x0040000000000000UL ) >> 45U ) | ( ( dw & 0x0000000000000200UL ) << 45U ) |
         ( ( dw & 0x0020000000000000UL ) >> 43U ) | ( ( dw & 0x0000000000000400UL ) << 43U ) |
         ( ( dw & 0x0010000000000000UL ) >> 41U ) | ( ( dw & 0x0000000000000800UL ) << 41U ) |
         ( ( dw & 0x0008000000000000UL ) >> 39U ) | ( ( dw & 0x0000000000001000UL ) << 39U ) |
         ( ( dw & 0x0004000000000000UL ) >> 37U ) | ( ( dw & 0x0000000000002000UL ) << 37U ) |
         ( ( dw & 0x0002000000000000UL ) >> 35U ) | ( ( dw & 0x0000000000004000UL ) << 35U ) |
         ( ( dw & 0x0001000000000000UL ) >> 33U ) | ( ( dw & 0x0000000000008000UL ) << 33U ) |
         ( ( dw & 0x0000800000000000UL ) >> 31U ) | ( ( dw & 0x0000000000010000UL ) << 31U ) |
         ( ( dw & 0x0000400000000000UL ) >> 29U ) | ( ( dw & 0x0000000000020000UL ) << 29U ) |
         ( ( dw & 0x0000200000000000UL ) >> 27U ) | ( ( dw & 0x0000000000040000UL ) << 27U ) |
         ( ( dw & 0x0000100000000000UL ) >> 25U ) | ( ( dw & 0x0000000000080000UL ) << 25U ) |
         ( ( dw & 0x0000080000000000UL ) >> 23U ) | ( ( dw & 0x0000000000100000UL ) << 23U ) |
         ( ( dw & 0x0000040000000000UL ) >> 21U ) | ( ( dw & 0x0000000000200000UL ) << 21U ) |
         ( ( dw & 0x0000020000000000UL ) >> 19U ) | ( ( dw & 0x0000000000400000UL ) << 19U ) |
         ( ( dw & 0x0000010000000000UL ) >> 17U ) | ( ( dw & 0x0000000000800000UL ) << 17U ) |
         ( ( dw & 0x0000008000000000UL ) >> 15U ) | ( ( dw & 0x0000000001000000UL ) << 15U ) |
         ( ( dw & 0x0000004000000000UL ) >> 13U ) | ( ( dw & 0x0000000002000000UL ) << 13U ) |
         ( ( dw & 0x0000002000000000UL ) >> 11U ) | ( ( dw & 0x0000000004000000UL ) << 11U ) |
         ( ( dw & 0x0000001000000000UL ) >>  9U ) | ( ( dw & 0x0000000008000000UL ) <<  9U ) |
         ( ( dw & 0x0000000800000000UL ) >>  7U ) | ( ( dw & 0x0000000010000000UL ) <<  7U ) |
         ( ( dw & 0x0000000400000000UL ) >>  5U ) | ( ( dw & 0x0000000020000000UL ) <<  5U ) |
         ( ( dw & 0x0000000200000000UL ) >>  3U ) | ( ( dw & 0x0000000040000000UL ) <<  3U ) |
         ( ( dw & 0x0000000100000000UL ) >>  1U ) | ( ( dw & 0x0000000080000000UL ) <<  1U ) ;
    *( field + i ) = dw;
  }
}


void check_reflect( uint8_t* buf, uint32_t n, uint8_t reflect )
{
  if( reflect )
  {
    reflect_bits_8( buf, n );
  }
}


int32_t walk_file( uint8_t** buf, ssize_t buf_len, 
                   const char* file, uint8_t* more_fragments )
{
  static uint8_t first_call = 0xFF;
  static uint8_t reset = 0x00;
  static int fd = ( -1 );

  struct stat file_stat;
  uint64_t file_size_byte = 0;
  int32_t bytes_read = ( -1 );
  off_t cur_file_pos;
 
  /* the function parameters are only stored
   * the first time this function gets called. */
  static uint8_t** buf_local = NULL;
  static ssize_t buf_len_local = 0;
  static const char* file_local = NULL;

  if( reset )
  {
    first_call = 0xFF;
    reset = 0x00;

    free( *buf_local );
    *buf_local = NULL;

    buf_len_local = 0;
    file_local = NULL;
    
    bytes_read = 0;
    return bytes_read;
  }

  if( first_call )
  {
    first_call = 0x00;
    
    buf_local = buf;
    buf_len_local = buf_len;
    file_local = file;

    if( !( *buf_local = ( uint8_t* )malloc( buf_len_local * sizeof( uint8_t ) ) ) )
    {
      ( void )fprintf( stderr, "Failed to allocate workspace memory.\n" );
      exit( EXIT_FAILURE );
    }

    if( ( fd = open( file_local, O_RDONLY ) ) == ( -1 ) )
    {
      ( void )fprintf( stderr, "%s\n", strerror( errno ) );
      free( *buf_local );
      exit( EXIT_FAILURE );
    }

    if( stat( file_local, &file_stat ) )
    {
      ( void )fprintf( stderr, "%s\n", strerror( errno ) );
      free( *buf_local );
      exit( EXIT_FAILURE );
    }
    file_size_byte = file_stat.st_size;
    ( void )fprintf( stdout, "\nThe input file has a size of %ld bytes.\n", 
                             file_size_byte );
  }

  bytes_read = ( int32_t )read( fd, ( void* )( *buf_local ), buf_len_local );

  cur_file_pos = lseek( fd, 0, SEEK_CUR );
  if( cur_file_pos == lseek( fd, 0, SEEK_END ) )
  {
    /* we are at the end of the file and can free resources */
    ( void )close( fd );
    fd = ( -1 );
    reset = 0xFFU;
    *more_fragments = 0x00U;
  }
  else
  {
    ( void )lseek( fd, cur_file_pos, SEEK_SET );
  }

  return bytes_read;
}

