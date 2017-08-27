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
/* #define FILE_BUF_SIZE ( 128UL * 1024UL * 1024UL ) */
#define FILE_BUF_SIZE ( 1UL )

/* CRC polynomial coefficients */
#define POLY_3  0x000000000000000BUL
#define POLY_8  0x000000000000000BUL
#define POLY_16 0x000000000000000BUL
#define POLY_32 0x000000000000000BUL
#define POLY_64 0x000000000000000BUL

#define BYTES_TO_BIT 8UL


static uint8_t polynomial_degree = 0x00;
/* the +1 is to assure that the path string is always 0 terminated. */
static uint8_t file[ PATH_MAX + 1 ] = { 0x00 };


static void parse_args( int argc, char** argv );
static void print_usage( FILE* out );
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
	uint8_t* file_buf = NULL;
  int fd = ( -1 );
  struct stat file_stat;
  uint64_t file_size_byte = 0;
  uint64_t file_size_bit  = 0;
	
	if( !( file_buf = ( uint8_t* )malloc( FILE_BUF_SIZE * sizeof( uint8_t ) ) ) )
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

  return;
	exit_fail:
    free( ( void* )file_buf );
    file_buf = NULL;
    ( void )close( fd );
    exit( EXIT_FAILURE );
}


int main( int argc, char** argv )
{
  parse_args( argc, argv );
  calculate_crc();
  return EXIT_SUCCESS;
}

