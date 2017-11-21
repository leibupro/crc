/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <pl@vqe.ch> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   P. Leibundgut
 * ----------------------------------------------------------------------------
 *
 * File:      main.c
 * 
 *
 * Purpose:   Program entry point for the 
 *            crc calculation program. Parse
 *            the arguments which were
 *            passed to this program.
 *
 * 
 * Remarks:   -
 *
 *
 * Date:      09/2017 
 * 
 */

#include <util.h>
#include <crcbit.h>
#include <crcbyte.h>

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <linux/limits.h>

#define DEF_CRC_MODE  32
#define DEF_OPT_LEVEL  0


static uint8_t polynomial_degree = 0x00;
/* the +1 is to assure that the path string is always 0 terminated. */
static uint8_t file[ PATH_MAX + 1 ] = { 0x00 };
static uint8_t optimize_level = 0x00;


static void parse_args( int argc, char** argv );
static void print_usage( FILE* out );


static void print_usage( FILE* out )
{
  ( void )fprintf( out, "\nUsage: \n"
                        " crc [-w crc_polynomial_degree] -f ./valid/file/path\n\n"
                        "=====================================================\n"
                        " Example: crc -w 32 -f /boot/vmlinuz-4.9.0-3-amd64\n" 
                        "=====================================================\n\n" );
  /* 
   * Splitted into two fprintf calls because of the:
   * error: string length ‘n’ is greater than the length ‘509’ 
   * ISO C90 compilers are required to support [-Woverlength-strings]
   */
  ( void )fprintf( out, "   -w   CRC Polynomial degree / Checksum width\n"
                        "        Default vlaue is 32 (CRC32).\n" 
                        "   -f   Valid path (relative or absolute)\n"
                        "        to an input file.\n"
                        "   -o   0 (default):\n"
                        "        No optimisation. Process the input stream\n" 
                        "        bitwise. (Shift register approach)\n"
                        "        1:\n"
                        "        Process bytewise\n"
                        "        2:\n"
                        "        Process bytewise with lookup table.\n\n" );
}


static void parse_args( int argc, char** argv )
{
  int option              = 0;
  long parsed_number      = 0;
  uint8_t file_count      = 0;
  uint8_t ignore_file     = 0;
  uint8_t poly_count      = 0;
  uint8_t ignore_poly     = 0;
  uint8_t optimize_count  = 0;
  uint8_t ignore_optimize = 0;

  if( argc <= 1 )
  {
    goto parse_fail;
  }

  while( ( option = getopt( argc, argv, "w:f:o:" ) ) != -1 )
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
      case 'o':
      {
        if( !ignore_optimize )
        {
          parsed_number = try_strtol( optarg );
          switch( parsed_number )
          {
            /* supported optimisation levels by now */
            case 0:
            case 1:
            case 2:
              optimize_level = ( uint8_t )parsed_number;
              optimize_count++;
              break;
          }
          ignore_optimize++;
        }
        else
        {
          ( void )fprintf( stdout, "Ignoring further polynomial degree arguments.\n" );
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
  if( !optimize_count )
  {
    optimize_level = ( uint8_t )DEF_OPT_LEVEL;
    ( void )fprintf( stdout, "No optimize level was specified. Using the default %d.\n"
                             "valid optimize levels: [ 0 | 1 | 2 ]\n", DEF_OPT_LEVEL );
  }
  return;
  parse_fail:
    print_usage( stderr );
    exit( EXIT_FAILURE );
}


int main( int argc, char** argv )
{
  parse_args( argc, argv );
  switch( optimize_level )
  {
    case 0:
      calculate_crc_from_file_bitwise( ( char* const )&file[ 0 ], 
                                       polynomial_degree );
      break;
    case 1:
      calculate_crc_from_file_bytewise( ( char* const )&file[ 0 ], 
                                        polynomial_degree );
      break;
    case 2:
      calculate_crc_from_file_bytewise_lut( ( char* const )&file[ 0 ], 
                                            polynomial_degree );
      break;
  }
  return EXIT_SUCCESS;
}

