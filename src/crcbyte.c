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


#define OPT_LEVEL_NONE 0U
#define OPT_LEVEL_LUT  1U

#define CRC_3_LEN  1U /* unit is bytes */
#define CRC_8_LEN  1U /* unit is bytes */
#define CRC_16_LEN 2U /* unit is bytes */
#define CRC_32_LEN 4U /* unit is bytes */
#define CRC_64_LEN 8U /* unit is bytes */


static crc_param_t polynomial_3  = CRC_3_POLY_PARAM;
static crc_param_t polynomial_8  = CRC_8_POLY_PARAM;
static crc_param_t polynomial_16 = CRC_16_POLY_PARAM;
static crc_param_t polynomial_32 = CRC_32_POLY_PARAM;
static crc_param_t polynomial_64 = CRC_64_POLY_PARAM;

static crc_param_t polynomial;

static void ( *crc_calc_func )( const char*, uint8_t ) = NULL;


static void init_polynomial_even( uint8_t degree );
static void calculate_crc16( const char* file, uint8_t opt_level );

void crc16_algorithm( uint8_t* data, const uint32_t len,
                      const crc_param_t* crc_params, uint16_t* p_crc,
                      uint8_t more_fragments );


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
      crc_calc_func = &calculate_crc16;
      break;
    case 32:
      polynomial = polynomial_32;
      break;
    case 64:
      polynomial = polynomial_64;
      break;
  }
}


void crc16_algorithm( uint8_t* data, const uint32_t len,
                      const crc_param_t* crc_params, uint16_t* p_crc,
                      uint8_t more_fragments )
{
  static uint8_t first_call = 0xFF;
  uint16_t poly = 0x0000;
  uint16_t crc  = 0x0000;
  uint32_t i;
  uint8_t j = 0x00, cur_byte = 0x00;

  if( len >= UINT32_MAX )
  {
    ( void )fprintf( stderr, "Invalid nuber of bytes passed to crc algorithm.\n" );
    return;
  }

  poly = crc_params->coeff.u_16;
  crc  = *p_crc;
    
  if( crc_params->reflect_input )
  {
    reflect_bits_8( data, len );
  }

  if( first_call )
  {
    first_call = 0x00;
    crc ^= crc_params->initial_xor.u_16;
  }

  for( i = 0; i < len; i++ )
  {
    cur_byte = *( data + i );
    crc ^= ( ( uint16_t )cur_byte ) << 8;

    for( j = 0; j < 8; j++ )
    {
      if( crc & 0x8000 )
      {
        crc = ( crc << 1 ) ^ poly;
      }
      else
      {
        crc <<= 1;
      }
    }
  }
  
  if( !more_fragments )
  {
    crc ^= crc_params->final_xor.u_16;

    if( crc_params->reflect_remainder )
    {
      reflect_bits_16( &crc, 1U );
    }
  }

  *p_crc = crc;
}


static void calculate_crc16( const char* file, uint8_t opt_level )
{
  int32_t  bytes_read     = 0;
  uint8_t* buf            = NULL;
  ssize_t  buf_len        = ( ssize_t )FILE_BUF_SIZE;
  uint16_t crc16          = 0x0000U;
  uint8_t  more_fragments = 0xFFU;
  
  while( ( bytes_read = walk_file( &buf, buf_len, file, &more_fragments ) ) )
  {
    ( void )fprintf( stdout, "Bytes read: %d\n", bytes_read );
    switch( opt_level )
    {
      case OPT_LEVEL_NONE:
        crc16_algorithm( buf, ( const uint32_t )bytes_read, 
                         ( const crc_param_t* )&polynomial, &crc16, 
                         more_fragments );
        break;
      case OPT_LEVEL_LUT:
        break;
    }
  }
  ( void )fprintf( stdout, "CRC16: %#06x\n", crc16 );
}


void calculate_crc_from_file_bytewise( const char* file, 
                                       uint8_t polynomial_degree )
{
  init_polynomial_even( polynomial_degree );
  crc_calc_func( file, OPT_LEVEL_NONE );
}


void calculate_crc_from_file_bytewise_lut( const char* file, 
                                           uint8_t polynomial_degree )
{
  char* foo;
  init_polynomial_even( polynomial_degree );
  foo = ( char* )file;
  foo = foo;
}

