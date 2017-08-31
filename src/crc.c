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

#include <linux/limits.h>

#define DEF_CRC_MODE 32
#define FILE_BUF_SIZE ( 64UL * 1024UL * 1024UL )

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

static uint8_t poly_3[]       = { 0xB0U };
static uint8_t poly_3_mask[]  = { 0xE0U };

static uint8_t poly_8[]       = { 0xEAU, 0x80U };
static uint8_t poly_8_mask[]  = { 0xFFU, 0x80U };

/* CRC-16-CCITT */
static uint8_t poly_16[]      = { 0x88U, 0x10U, 0x80U };
static uint8_t poly_16_mask[] = { 0xFFU, 0xFFU, 0x80U };

/* CRC-32 */
static uint8_t poly_32[]      = { 0x82U, 0x60U, 0x8EU, 0xDBU, 0x80U };
static uint8_t poly_32_mask[] = { 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0x80U };

/* CRC-64-ISO */
static uint8_t poly_64[]      = { 0x80U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x0DU, 0x80U };
static uint8_t poly_64_mask[] = { 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0x80U };

/* polynomials can be accessed with the polynomial_degree as index */
static uint8_t* polynomials[ 65 ][ 2 ];

#define BYTES_TO_BIT 8UL


static uint8_t polynomial_degree = 0x00;
/* the +1 is to assure that the path string is always 0 terminated. */
static uint8_t file[ PATH_MAX + 1 ] = { 0x00 };


static void parse_args( int argc, char** argv );
static void print_usage( FILE* out );
static void init_polynomials( void );
static void free_resources( int fd, void* file_buf, void* poly_buf, void* mask_buf );
static void calculate_crc( void );


static void print_usage( FILE* out )
{
  ( void )fprintf( out, "\nUsage: \n"
                        " crc [-w crc_polynomial_degree] -f ./valid/file/path\n\n"
                        "=====================================================\n"
                        " Example: crc -w 32 -f /boot/vmlinuz-4.9.0-3-amd64\n" 
                        "=====================================================\n\n"
                        "   -w   CRC Polynomial degree / Checksum width\n"
                        "        Default vlaue is 32 (CRC32).\n" 
                        "   -f   Valid path (relative or absolute)\n"
                        "        to an input file.\n\n" );
}


static void init_polynomials( void )
{
  polynomials[  3 ][ 0 ] = &poly_3[ 0 ];
  polynomials[  3 ][ 1 ] = &poly_3_mask[ 0 ];
  
  polynomials[  8 ][ 0 ] = &poly_8[ 0 ];
  polynomials[  8 ][ 1 ] = &poly_8_mask[ 0 ];
  
  polynomials[ 16 ][ 0 ] = &poly_16[ 0 ];
  polynomials[ 16 ][ 1 ] = &poly_16_mask[ 0 ];
  
  polynomials[ 32 ][ 0 ] = &poly_32[ 0 ];
  polynomials[ 32 ][ 1 ] = &poly_32_mask[ 0 ];
  
  polynomials[ 64 ][ 0 ] = &poly_64[ 0 ];
  polynomials[ 64 ][ 1 ] = &poly_64_mask[ 0 ];
}


static void free_resources( int fd, void* file_buf, void* poly_buf, void* mask_buf )
{
  free( file_buf );
  free( poly_buf );
  free( mask_buf );
  ( void )close( fd );
}


static void parse_args( int argc, char** argv )
{
  int option          = 0;
  long parsed_number  = 0;
  uint8_t file_count  = 0;
  uint8_t ignore_file = 0;
  uint8_t poly_count  = 0;
  uint8_t ignore_poly = 0;

  if( argc <= 1 )
  {
    goto parse_fail;
  }

  while( ( option = getopt( argc, argv, "w:f:" ) ) != -1 )
  {
    switch( option )
    {
      case 'w':
      {
        if( !ignore_poly )
        {
          parsed_number = try_strtol( optarg );
          switch( parsed_number )
          {
            /* supported polynomials by now */
            case  3:
            case  8:
            case 16:
            case 32:
            case 64:
              polynomial_degree = ( uint8_t )parsed_number;
              poly_count++;
              break;
          }
          ignore_poly++;
        }
        else
        {
          ( void )fprintf( stdout, "Ignoring further polynomial degree arguments.\n" );
        }
        break;
      }
      case 'f':
      {
        if( !ignore_file )
        {
          ( void )strncpy( ( char* )&file[ 0 ], optarg, PATH_MAX );
          if( !access( ( const char* )&file[ 0 ], F_OK ) )
          {
            file_count++;
          }
          ignore_file++;
        }
        else
        {
          ( void )fprintf( stdout, "Ignoring further file arguments.\n" );
        }
        break;
      }
      default:
      {
        goto parse_fail;
      }
    }
  }
  if( !file_count )
  {
    ( void )fprintf( stderr, "One input file that exists (valid path) must be specified.\n" );
    goto parse_fail;
  }
  if( !poly_count )
  {
    polynomial_degree = ( uint8_t )DEF_CRC_MODE;
    ( void )fprintf( stdout, "you specified no or an unsupported polynomial degree.\n"
                             "polynomial degree / checksum width must be: [ 3 | 8 | 16 | 32 | 64 ]\n"
                             "using the default polynomial degree: %d\n", DEF_CRC_MODE );
  }
  return;
  parse_fail:
    print_usage( stderr );
    exit( EXIT_FAILURE );
}


static void calculate_crc( void )
{
	uint8_t* file_buf     = NULL;
	uint8_t* poly_buf     = NULL;
  uint8_t* mask_buf     = NULL;

  uint8_t* realloc_file = NULL;
  uint8_t* realloc_poly = NULL;
  uint8_t* realloc_mask = NULL;
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

  checksum_size = 
    ( polynomial_degree / BYTES_TO_BIT == 0 ) ? 1 : ( polynomial_degree / BYTES_TO_BIT );
  poly_bytes = 
    ( polynomial_degree / BYTES_TO_BIT == 0 ) ? 1 : ( checksum_size + 1 );
	
	if( !( file_buf = ( uint8_t* )malloc( file_buf_size * sizeof( uint8_t ) ) ) ||
	    !( poly_buf = ( uint8_t* )calloc( 1, file_buf_size ) ) ||
	    !( mask_buf = ( uint8_t* )calloc( 1, file_buf_size ) ) )
  {
		( void )fprintf( stderr, "Failed to allocate workspace memory.\n" );
    exit( EXIT_FAILURE );
  }

  if( ( fd = open( ( const char* )file, O_RDONLY ) ) == ( -1 ) )
  {
    ( void )fprintf( stderr, "%s\n", strerror( errno ) );
    goto exit_fail;
  }

  if( stat( ( const char* )file, &file_stat ) )
  {
    ( void )fprintf( stderr, "%s\n", strerror( errno ) );
    goto exit_fail;
  }
  file_size_byte = file_stat.st_size;
  file_size_bit  = file_size_byte * BYTES_TO_BIT;
  ( void )fprintf( stdout, "\nThe input file has a size of %ld bytes (%ld bit).\n", 
                           file_size_byte,
                           file_size_bit );

  if( file_buf_size <= ( uint32_t )poly_bytes )
  {
    ( void )fprintf( stderr, "File buffer must be greater than polynomial size.\n" );
    goto exit_fail;
  }

  if( file_size_byte <= file_buf_size )
  {
    first_last = 0xFF;
  }

  do
  {
    if( first )
    {
      bytes_read = read( fd, ( void* )file_buf, file_buf_size );
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
      ( void )memmove( ( void* )mask_buf, 
                       ( void* )( mask_buf + ( file_buf_size - poly_bytes ) ), 
                       poly_bytes );

      bytes_read = read( fd, 
                         ( void* )( file_buf + poly_bytes ), 
                         ( file_buf_size - poly_bytes ) );
    }

    /* we are at the end and can add the checksum bytes */
    cur_file_pos = lseek( fd, 0, SEEK_CUR );
    if( cur_file_pos == lseek( fd, 0, SEEK_END ) )
    {
      if( first_last )
      {
        realloc_size = ( ssize_t )( bytes_read + checksum_size );
        bytes_to_process = bytes_read;
      }
      else
      {
        realloc_size = ( ssize_t )( poly_bytes + bytes_read + checksum_size );
        bytes_to_process = poly_bytes + bytes_read;
      }
      if( !( realloc_file = ( uint8_t* )realloc( ( void* )file_buf, realloc_size ) ) ||
          !( realloc_poly = ( uint8_t* )realloc( ( void* )poly_buf, realloc_size ) ) ||
          !( realloc_mask = ( uint8_t* )realloc( ( void* )mask_buf, realloc_size ) ) )
      {
        ( void )fprintf( stderr, "Failed to reallocate memory.\n" );
        goto exit_fail;
      }
      file_buf = realloc_file;
      poly_buf = realloc_poly;
      mask_buf = realloc_mask;
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
  } while( file_walker );

  free_resources( fd, ( void* )file_buf, ( void* )poly_buf, ( void* )mask_buf );
  return;

	exit_fail:
    free_resources( fd, ( void* )file_buf, ( void* )poly_buf, ( void* )mask_buf );
    exit( EXIT_FAILURE );
}


int main( int argc, char** argv )
{
  parse_args( argc, argv );
  init_polynomials();
  calculate_crc();
  return EXIT_SUCCESS;
}

