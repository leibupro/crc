#include <util.h>

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <linux/limits.h>

#define DEF_CRC_MODE 32

static uint8_t polynomial_degree = 0x00;
/* the +1 is to assure that the path string is always 0 terminated. */
static uint8_t file[ PATH_MAX + 1 ] = { 0x00 };


static void parse_args( int argc, char** argv );
static void print_usage( FILE* out );


static void print_usage( FILE* out )
{
  ( void )fprintf( out, "print usage ...\n" );
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


int main( int argc, char** argv )
{
  parse_args( argc, argv );
  return EXIT_SUCCESS;
}

