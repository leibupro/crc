/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <pl@vqe.ch> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   P. Leibundgut
 * ----------------------------------------------------------------------------
 *
 * File:      polytstr.c
 * 
 *
 * Purpose:   This program reports the distribution of
 *            the CRC checksums which are generated over
 *            a data field. Due to the large input
 *            possibilities (permute all bits), the
 *            execution should be performed with
 *            multiple threads. ... Or single-threaded,
 *            if you have more time.
 *            
 *
 * Remarks:   - This test program requires the crc library,
 *              which does not reside in the gcc stdlibs.
 *
 *            - link against -lcrc and -lpthread
 *
 *            - For now we have CRC16 polynomials
 *              applied on 6 byte input fields:
 *              => 2^48 inputs which return a 
 *                 checksum in the range: 
 *                 [ 0, ..., 2^16-1 ]
 *
 *
 * Date:      01/2018 
 * 
 */

#include <crctypes.h>
#include <crcparam_even.h>
#include <crcapi.h>

#include <stdint.h>
#include <stdio.h>

#define NUM_THREADS 2U
#define INPUT_BYTES 6U

static crc_param_t crc_params = CRC_16_CCITT_FALSE;

int main( void )
{
  uint8_t foobar[ INPUT_BYTES ] = { 0x1FU, 0x39U, 0xC3U, 0x56U, 0x00U, 0x0FU };
  uint16_t crc16 = 0x0000U;

  crc16_algorithm( &foobar[ 0U ], ( const uint32_t )INPUT_BYTES,
                   ( const crc_param_t* )&crc_params, &crc16,
                   0x00U );

  ( void )fprintf( stdout, "CRC16: %#06x\n", crc16 );

  return 0;
}

