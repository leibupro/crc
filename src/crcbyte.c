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

static uint16_t lut_crc_16[ 0x100U ] = { 0x0000U };


static void init_polynomial_even( uint8_t degree );
static void calculate_crc16( const char* file, uint8_t opt_level );

void crc16_algorithm( uint8_t* data, const uint32_t len,
                      const crc_param_t* crc_params, uint16_t* p_crc,
                      uint8_t first_call,
                      uint8_t more_fragments );

void crc16_algorithm_lut( uint8_t* data, const uint32_t len,
                          const crc_param_t* crc_params, uint16_t* p_crc,
                          uint8_t first_call,
                          uint8_t more_fragments );

void init_lut_crc_16( const crc_param_t* crc_params );


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


void init_lut_crc_16( const crc_param_t* crc_params )
{
  uint16_t i;
  uint16_t cur_byte;
  uint16_t poly;
  uint8_t j;

  poly = crc_params->coeff.u_16;

  for( i = 0U; i < 0x100U; i++ )
  {
    cur_byte = ( i << 8U );

    for( j = 0U; j < 8U; j++ )
    {
      if( cur_byte & 0x8000U )
      {
        cur_byte <<= 1U;
        cur_byte  ^= poly;
      }
      else
      {
        cur_byte <<= 1U;
      }
    }
    lut_crc_16[ i ] = cur_byte;
  }
}


void crc16_algorithm_lut( uint8_t* data, const uint32_t len,
                          const crc_param_t* crc_params, uint16_t* p_crc,
                          uint8_t first_call,
                          uint8_t more_fragments )
{
  uint32_t i;
  uint16_t crc = 0x0000U;
  uint8_t cur_byte;
  uint8_t position;

  if( len >= UINT32_MAX )
  {
    ( void )fprintf( stderr, "Invalid number of bytes passed to crc algorithm.\n" );
    return;
  }
  
  crc = *p_crc;
  
  if( crc_params->reflect_input )
  {
    reflect_bits_8( data, len );
  }
  
  if( first_call )
  {
    crc ^= crc_params->initial_xor.u_16;
  }

  for( i = 0U; i < len; i++ )
  {
    cur_byte = *( data + i );
    position = ( ( uint8_t )( crc >> 8U ) ) ^ cur_byte;
    crc = ( crc << 8U ) ^ lut_crc_16[ position ];
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


void crc16_algorithm( uint8_t* data, const uint32_t len,
                      const crc_param_t* crc_params, uint16_t* p_crc,
                      uint8_t first_call,
                      uint8_t more_fragments )
{
  uint16_t poly = 0x0000U;
  uint16_t crc  = 0x0000U;
  uint32_t i;
  uint8_t j = 0x00U, cur_byte = 0x00U;

  if( len >= UINT32_MAX )
  {
    ( void )fprintf( stderr, "Invalid number of bytes passed to crc algorithm.\n" );
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
  uint8_t  first_call     = 0xFFU;
  uint8_t  more_fragments = 0xFFU;

  if( opt_level == OPT_LEVEL_LUT )
  {
    init_lut_crc_16( ( const crc_param_t* )&polynomial );
  }
  
  while( ( bytes_read = walk_file( &buf, buf_len, file, &more_fragments ) ) )
  {
    ( void )fprintf( stdout, "Bytes read: %d\n", bytes_read );
    switch( opt_level )
    {
      case OPT_LEVEL_NONE:
        crc16_algorithm( buf, ( const uint32_t )bytes_read, 
                         ( const crc_param_t* )&polynomial, &crc16,
                         first_call,
                         more_fragments );
        break;
      case OPT_LEVEL_LUT:
        crc16_algorithm_lut( buf, ( const uint32_t )bytes_read, 
                             ( const crc_param_t* )&polynomial, &crc16,
                             first_call,
                             more_fragments );
        break;
    }
    if( first_call )
    {
      first_call = 0x00U;
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
  init_polynomial_even( polynomial_degree );
  crc_calc_func( file, OPT_LEVEL_NONE );
}

