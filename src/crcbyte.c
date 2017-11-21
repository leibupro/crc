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
#include <crc.h>
#include <crcbyte.h>

#include <stdint.h>

/* Polynimial coefficients where the most sgnificant bit is omitted */

/* CRC-3 example from wikipedia */
static uint8_t  poly_3_even     = 0x03U;
static uint8_t  poly_3_even_ix  = 0x00U;
static uint8_t  poly_3_even_fx  = 0x00U;

/* CRC-8-CCITT */
static uint8_t  poly_8_even     = 0x07U;
static uint8_t  poly_8_even_ix  = 0x00U;
static uint8_t  poly_8_even_fx  = 0x00U;

/* CRC-16-CCITT (zero) */
static uint16_t poly_16_even    = 0x1021U;
static uint16_t poly_16_even_ix = 0x0000U;
static uint16_t poly_16_even_fx = 0x0000U;

/* CRC-32 */
static uint32_t poly_32_even    = 0x04C11DB7U;
static uint32_t poly_32_even_ix = 0xFFFFFFFFU;
static uint32_t poly_32_even_fx = 0xFFFFFFFFU;

/* CRC-64-ISO */
static uint64_t poly_64_even    = 0x000000000000001BU;
static uint64_t poly_64_even_ix = 0x0000000000000000U;
static uint64_t poly_64_even_fx = 0x0000000000000000U;

crc_param_t polynomial;


static void init_polynomial_even( uint8_t degree );


static void init_polynomial_even( uint8_t degree )
{
  polynomial.degree = degree;

  switch( degree )
  {
    case  3:
      polynomial.coeff.u_8         = &poly_3_even;
      polynomial.initial_xor.u_8   = &poly_3_even_ix;
      polynomial.final_xor.u_8     = &poly_3_even_fx;
      polynomial.reflect_input     = 0x00;
      polynomial.reflect_remainder = 0x00;
      break;
    case  8:
      polynomial.coeff.u_8         = &poly_8_even;
      polynomial.initial_xor.u_8   = &poly_8_even_ix;
      polynomial.final_xor.u_8     = &poly_8_even_fx;
      polynomial.reflect_input     = 0x00;
      polynomial.reflect_remainder = 0x00;
      break;
    case 16:
      polynomial.coeff.u_16        = &poly_16_even;
      polynomial.initial_xor.u_16  = &poly_16_even_ix;
      polynomial.final_xor.u_16    = &poly_16_even_fx;
      polynomial.reflect_input     = 0x00;
      polynomial.reflect_remainder = 0x00;
      break;
    case 32:
      polynomial.coeff.u_32        = &poly_32_even;
      polynomial.initial_xor.u_32  = &poly_32_even_ix;
      polynomial.final_xor.u_32    = &poly_32_even_fx;
      polynomial.reflect_input     = 0xFF;
      polynomial.reflect_remainder = 0xFF;
      break;
    case 64:
      polynomial.coeff.u_64        = &poly_64_even;
      polynomial.initial_xor.u_64  = &poly_64_even_ix;
      polynomial.final_xor.u_64    = &poly_64_even_fx;
      polynomial.reflect_input     = 0x00;
      polynomial.reflect_remainder = 0x00;
      break;
  }
}


void calculate_crc_from_file_bytewise( char* const file, 
                                       uint8_t polynomial_degree )
{
  char* foo;
  init_polynomial_even( polynomial_degree );
  foo = file;
  foo = foo;
}


void calculate_crc_from_file_bytewise_lut( char* const file, 
                                           uint8_t polynomial_degree )
{
  char* foo;
  init_polynomial_even( polynomial_degree );
  foo = file;
  foo = foo;
}

