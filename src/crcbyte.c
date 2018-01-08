/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <pl@vqe.ch> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   P. Leibundgut
 * ----------------------------------------------------------------------------
 *
 * File:      crcbyte.c
 * 
 *
 * Purpose:   This module holds all the
 *            necessary functions to calculate 
 *            CRC checksums bytewise.
 *
 * 
 * Remarks:   - By now CRC32 polynomials are
 *              tested and fully functional.
 *
 *            - The program crunches the input
 *              streams byte by byte.
 *
 *
 * Date:      11/2017 
 * 
 */

#include <crctypes.h>
#include <crc.h>
#include <crcparam_even.h>
#include <crcbyte.h>
#include <util.h>

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>


static crc_param_t polynomial_3  = CRC_3_POLY_PARAM;
static crc_param_t polynomial_8  = CRC_8_POLY_PARAM;
static crc_param_t polynomial_16 = CRC_16_POLY_PARAM;
static crc_param_t polynomial_32 = CRC_32_POLY_PARAM;
static crc_param_t polynomial_64 = CRC_64_POLY_PARAM;

static crc_param_t polynomial;


static void init_polynomial_even( uint8_t degree );
static uint16_t calculate_crc16( const char* file );


static void init_polynomial_even( uint8_t degree )
{
  switch( degree )
  {
    case  3:
      polynomial = polynomial_3;
      break;
    case  8:
      polynomial = polynomial_8;
      break;
    case 16:
      polynomial = polynomial_16;
      break;
    case 32:
      polynomial = polynomial_32;
      break;
    case 64:
      polynomial = polynomial_64;
      break;
  }
}


static uint16_t calculate_crc16( const char* file )
{
  int32_t  bytes_read = 0;
  uint8_t* buf        = NULL;
  ssize_t  buf_len    = FILE_BUF_SIZE;

  while( ( bytes_read = walk_file( &buf, buf_len, file ) ) )
  {
    ( void )fprintf( stdout, "Bytes read: %d\n", bytes_read );
  }
  return 0xFFFF;
}


void calculate_crc_from_file_bytewise( const char* file, 
                                       uint8_t polynomial_degree )
{
  init_polynomial_even( polynomial_degree );
  calculate_crc16( file );
}


void calculate_crc_from_file_bytewise_lut( const char* file, 
                                           uint8_t polynomial_degree )
{
  char* foo;
  init_polynomial_even( polynomial_degree );
  foo = ( char* )file;
  foo = foo;
}

