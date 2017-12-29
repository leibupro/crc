/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <pl@vqe.ch> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   P. Leibundgut
 * ----------------------------------------------------------------------------
 *
 * File:      crcbit.c
 * 
 *
 * Purpose:   This module holds all the
 *            necessary functions to calculate 
 *            CRC checksums bitwise.
 *
 * 
 * Remarks:   - By now CRC32 polynomials are
 *              tested and fully functional.
 *
 *            - The program crunches the input
 *              streams bit by bit. This
 *              can be optimized for example
 *              with a lookup table.
 *              A feature that should be
 *              implemented as soon as
 *              all the planned polynomials
 *              are tested and functional.
 *
 *
 * Date:      09/2017 
 * 
 */

#include <crc.h>
#include <crcbit.h>
#include <util.h>

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BYTES_TO_BIT   8UL
#define RIGHT_MOST_BIT 0x01U
#define LEFT_MOST_BIT  0x80U
#define MODULO_8_MASK  0x0000000000000007UL


/* CRC polynomial coefficients 
 * OK, these polynomials were taken from this source: 
 * https://en.wikipedia.org/wiki/Polynomial_representations_of_cyclic_redundancy_checks
 * The omission of the high-order bit was not applied here.
 * We have an extra byte for the odd bit.
 *
 * E. g.:
 * CRC32, given polynomial coefficients: 0x04C11DB7
 * Added a 1 for the first bit (x^32): 0x104C11DB7
 * Left shift of seven bit: 0x82608EDB80
 */

/* CRC-3 example from wikipedia */
static uint8_t poly_3_odd[]    = { 0xB0U };
static uint8_t poly_3_odd_ix[] = { 0x00U };
static uint8_t poly_3_odd_fx[] = { 0x00U };

/* CRC-8-CCITT */
static uint8_t poly_8_odd[]     = { 0x83U, 0x80U };
static uint8_t poly_8_odd_ix[]  = { 0x00U };
static uint8_t poly_8_odd_fx[]  = { 0x00U };

/* CRC-16-CCITT (zero) */
static uint8_t poly_16_odd[]    = { 0x88U, 0x10U, 0x80U };
static uint8_t poly_16_odd_ix[] = { 0xFFU, 0xFFU };
static uint8_t poly_16_odd_fx[] = { 0x00U, 0x00U };

/* CRC-32 */
static uint8_t poly_32_odd[]    = { 0x82U, 0x60U, 0x8EU, 0xDBU, 0x80U };
static uint8_t poly_32_odd_ix[] = { 0xFFU, 0xFFU, 0xFFU, 0xFFU };
static uint8_t poly_32_odd_fx[] = { 0xFFU, 0xFFU, 0xFFU, 0xFFU };

/* CRC-64-ISO */
static uint8_t poly_64_odd[]    = { 0x80U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x0DU, 0x80U };
static uint8_t poly_64_odd_ix[] = { 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U };
static uint8_t poly_64_odd_fx[] = { 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U };

crc_param_t polynomial;

static void init_polynomial_odd( uint8_t degree );
static void free_resources( int fd, void* file_buf, void* poly_buf );

static void 
crunch( uint8_t* file_buf, uint8_t* poly_buf, 
        uint32_t bytes_to_process, uint8_t poly_bytes, 
        uint8_t** remainder );

static inline void shift_one_right( uint8_t* field, uint8_t overflows );

static void print_crc_checksum( FILE* out, uint8_t checksum_bytes, 
                                uint8_t* remainder_location );
static void xor_bits_bytewise( uint8_t* field, uint8_t* xor_values, uint32_t n );


static void init_polynomial_odd( uint8_t degree )
{
  polynomial.degree = degree;

  switch( degree )
  {
    case  3:
      polynomial.coeff.u_8         = &poly_3_odd   [ 0 ];
      polynomial.initial_xor.u_8   = &poly_3_odd_ix[ 0 ];
      polynomial.final_xor.u_8     = &poly_3_odd_fx[ 0 ];
      polynomial.reflect_input     = 0x00;
      polynomial.reflect_remainder = 0x00;
      break;
    case  8:
      polynomial.coeff.u_8         = &poly_8_odd   [ 0 ];
      polynomial.initial_xor.u_8   = &poly_8_odd_ix[ 0 ];
      polynomial.final_xor.u_8     = &poly_8_odd_fx[ 0 ];
      polynomial.reflect_input     = 0x00;
      polynomial.reflect_remainder = 0x00;
      break;
    case 16:
      polynomial.coeff.u_8         = &poly_16_odd   [ 0 ];
      polynomial.initial_xor.u_8   = &poly_16_odd_ix[ 0 ];
      polynomial.final_xor.u_8     = &poly_16_odd_fx[ 0 ];
      polynomial.reflect_input     = 0x00;
      polynomial.reflect_remainder = 0x00;
      break;
    case 32:
      polynomial.coeff.u_8         = &poly_32_odd   [ 0 ];
      polynomial.initial_xor.u_8   = &poly_32_odd_ix[ 0 ];
      polynomial.final_xor.u_8     = &poly_32_odd_fx[ 0 ];
      polynomial.reflect_input     = 0xFF;
      polynomial.reflect_remainder = 0xFF;
      break;
    case 64:
      polynomial.coeff.u_8         = &poly_64_odd   [ 0 ];
      polynomial.initial_xor.u_8   = &poly_64_odd_ix[ 0 ];
      polynomial.final_xor.u_8     = &poly_64_odd_fx[ 0 ];
      polynomial.reflect_input     = 0x00;
      polynomial.reflect_remainder = 0x00;
      break;
  }
}


static void free_resources( int fd, void* file_buf, void* poly_buf )
{
  free( file_buf );
  free( poly_buf );
  ( void )close( fd );
}


static inline void shift_one_right( uint8_t* field, uint8_t overflows )
{
  uint8_t i, c = 0x00;

  for( i = overflows; i > 0; i-- )
  {
    *( field + i ) >>= 1U;
    c = ( *( field + ( i - 1 ) ) & RIGHT_MOST_BIT ) << 7U;
    *( field + i ) |= c;
  }

  *field >>= 1U;
}


static void 
crunch( uint8_t* file_buf, uint8_t* poly_buf, 
        uint32_t bytes_to_process, uint8_t poly_bytes,
        uint8_t** remainder )
{
  uint32_t byte_offset, i;
  uint8_t  bit_offset;
  uint8_t  first_bit = 0x80U;
  
  for( byte_offset = 0; byte_offset < bytes_to_process; byte_offset++ )
  {
    for( bit_offset = 0; bit_offset < 8; bit_offset++ )
    {
      if( *( file_buf + byte_offset ) & ( first_bit >> bit_offset ) )
      {
        for( i = byte_offset; i < ( byte_offset + poly_bytes ); i++ )
        {
          *( file_buf + i ) ^= *( poly_buf + i );
        }
      }
      shift_one_right( ( poly_buf + byte_offset ), poly_bytes );
    }
  }

  *remainder = ( file_buf + bytes_to_process );
}


static void print_crc_checksum( FILE* out, uint8_t checksum_bytes, 
                                uint8_t* remainder_location )
{
  int8_t i;

  ( void )fprintf( out, "\n\nCRC checksum: 0x" );
  if( polynomial.reflect_remainder )
  {
    for( i = ( int8_t )( checksum_bytes - 1 ); i >= 0; i-- )
    {
      ( void )fprintf( out, "%02x", *( remainder_location + i ) );
    }
  }
  else
  {
    for( i = 0; i < ( int8_t )checksum_bytes; i++ )
    {
      ( void )fprintf( out, "%02x", *( remainder_location + i ) );
    }
  }
  ( void )fprintf( out, "\n" );
}


static void xor_bits_bytewise( uint8_t* field, uint8_t* xor_values, uint32_t n )
{
  uint32_t i;

  for( i = 0; i < n; i++ )
  {
    *( field + i ) ^= *( xor_values + i );
  }
}


void calculate_crc_from_file_bitwise( char* const file, 
                                      uint8_t polynomial_degree )
{
  uint8_t* file_buf     = NULL;
  uint8_t* poly_buf     = NULL;

  uint8_t* realloc_file = NULL;
  uint8_t* realloc_poly = NULL;
  ssize_t realloc_size = 0;

  int fd = ( -1 );
  struct stat file_stat;
  uint64_t file_size_byte = 0;
  uint64_t file_size_bit  = 0;
  ssize_t bytes_read = 0;
  
  /* uint is bytes */
  uint8_t checksum_size = 0x00;

  /* uint is bytes */
  uint8_t poly_bytes = 0x00;

  uint32_t bytes_to_process = 0x00;
  
  uint32_t file_buf_size = FILE_BUF_SIZE;

  uint8_t first      = 0xFF;
  uint8_t first_last = 0x00;

  uint8_t file_walker = 0xFF;

  off_t cur_file_pos = 0;

  uint32_t file_piece_no = 0;

  uint8_t* remainder_location = NULL;
  
  init_polynomial_odd( polynomial_degree );

  checksum_size = 
    ( polynomial_degree / BYTES_TO_BIT == 0 ) ? 1 : ( polynomial_degree / BYTES_TO_BIT );
  poly_bytes = 
    ( polynomial_degree / BYTES_TO_BIT == 0 ) ? 1 : ( checksum_size + 1 );
  
  if( !( file_buf = ( uint8_t* )malloc( file_buf_size * sizeof( uint8_t ) ) ) ||
      !( poly_buf = ( uint8_t* )calloc( 1, file_buf_size ) ) )
  {
    ( void )fprintf( stderr, "Failed to allocate workspace memory.\n" );
    exit( EXIT_FAILURE );
  }

  if( ( fd = open( file, O_RDONLY ) ) == ( -1 ) )
  {
    ( void )fprintf( stderr, "%s\n", strerror( errno ) );
    goto exit_fail;
  }

  if( stat( file, &file_stat ) )
  {
    ( void )fprintf( stderr, "%s\n", strerror( errno ) );
    goto exit_fail;
  }
  file_size_byte = file_stat.st_size;
  file_size_bit  = file_size_byte * BYTES_TO_BIT;
  ( void )fprintf( stdout, "\nThe input file has a size of %ld bytes (%ld bit).\n", 
                           file_size_byte,
                           file_size_bit );

  if( !file_size_byte )
  {
    ( void )fprintf( stdout, "File size is zero bytes. Checksum is 0.\n" );
    goto exit_success;
  }

  if( file_buf_size <= ( uint32_t )poly_bytes )
  {
    printf( "%d\n", poly_bytes );
    ( void )fprintf( stderr, "File buffer must be greater than polynomial size.\n" );
    goto exit_fail;
  }

  if( file_size_byte <= file_buf_size )
  {
    first_last = 0xFF;
  }

  /* write according polynomial coefficients into buffer. */
  ( void )memcpy( ( void* )poly_buf, 
                  ( const void* )polynomial.coeff.u_8, 
                  poly_bytes );

  do
  {
    if( first )
    {
      bytes_read = read( fd, ( void* )file_buf, file_buf_size );
      check_reflect( file_buf, bytes_read, polynomial.reflect_input );
      xor_bits_bytewise( file_buf, polynomial.initial_xor.u_8, 
                         ( uint32_t )checksum_size );
      first = 0x00;
    }
    else
    {
      ( void )memmove( ( void* )file_buf, 
                       ( void* )( file_buf + ( file_buf_size - poly_bytes ) ), 
                       poly_bytes );
      ( void )memmove( ( void* )poly_buf, 
                       ( void* )( poly_buf + ( file_buf_size - poly_bytes ) ), 
                       poly_bytes );

      /* after moving the polynomial buffer to the start of the poly_buf,
       * the bytes at the end have to be zeroed. */
      ( void )memset( ( void* )( poly_buf + poly_bytes ), 0x00000000,
                      ( file_buf_size - poly_bytes ) );

      bytes_read = read( fd, 
                         ( void* )( file_buf + poly_bytes ), 
                         ( file_buf_size - poly_bytes ) );
      check_reflect( ( file_buf + poly_bytes ), bytes_read, polynomial.reflect_input );
    }

    /* we are at the end and can add the checksum bytes */
    cur_file_pos = lseek( fd, 0, SEEK_CUR );
    if( cur_file_pos == lseek( fd, 0, SEEK_END ) )
    {
      /* 
       * The + 1 in the realloc size is here so that we have enough
       * space available and the right shift during the last byte
       * will not segfault.
       */
      if( first_last )
      {
        realloc_size = ( ssize_t )( bytes_read + checksum_size + 1 );
        bytes_to_process = bytes_read;
      }
      else
      {
        realloc_size = ( ssize_t )( poly_bytes + bytes_read + checksum_size + 1 );
        bytes_to_process = poly_bytes + bytes_read;
      }
      if( !( realloc_file = ( uint8_t* )realloc( ( void* )file_buf, realloc_size ) ) ||
          !( realloc_poly = ( uint8_t* )realloc( ( void* )poly_buf, realloc_size ) ) )
      {
        ( void )fprintf( stderr, "Failed to reallocate memory.\n" );
        goto exit_fail;
      }
      file_buf = realloc_file;
      poly_buf = realloc_poly;
      file_walker = 0x00;
    }
    else
    {
      ( void )lseek( fd, cur_file_pos, SEEK_SET );
      bytes_to_process = file_buf_size - poly_bytes;
    }
    ( void )fprintf( stdout, "%5d: keep walking ..., bytes to process: %d\n", 
                             ++file_piece_no,
                             bytes_to_process );

    crunch( file_buf, poly_buf,
            bytes_to_process, poly_bytes, 
            &remainder_location );

  } while( file_walker );
  
  xor_bits_bytewise( remainder_location, polynomial.final_xor.u_8, 
                     ( uint32_t )checksum_size );

  check_reflect( remainder_location, checksum_size, polynomial.reflect_remainder );

  print_crc_checksum( stdout, checksum_size, remainder_location );

  /* just to be correct and consistent */
  goto exit_success;

  exit_success:
    free_resources( fd, ( void* )file_buf, ( void* )poly_buf );
    return;

  exit_fail:
    free_resources( fd, ( void* )file_buf, ( void* )poly_buf );
    exit( EXIT_FAILURE );
}

