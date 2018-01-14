/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <pl@vqe.ch> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   P. Leibundgut
 * ----------------------------------------------------------------------------
 *
 * File:      hamming.c
 * 
 *
 * Purpose:   Module to calculate the hamming
 *            distance of byte fields.
 *            
 *
 * Remarks:   -
 *
 *
 * Date:      01/2018 
 * 
 */

#include <hamming.h>
#include <stdint.h>
#include <stdio.h>

uint32_t get_hamming_distance( const uint8_t* field_a, 
                               const uint8_t* field_b, uint16_t len )
{
  uint32_t distance_sum = 0U;
  uint16_t i;
  uint8_t  cur_xor;
  uint8_t  cur_sum = 0x00U;

  if( len >= UINT16_MAX )
  {
    ( void )fprintf( stderr, "Invalid nuber of bytes passed to crc algorithm.\n" );
    return distance_sum;
  }

  for( i = 0U; i < len; i++ )
  {
    for( cur_xor = *( field_a + i ) ^ *( field_b + i ); 
         cur_xor; 
         cur_xor <<= 1U )
    {
      if( cur_xor & 0x80U )
      {
        cur_sum++;
      }
    }
    distance_sum += cur_sum;
    cur_sum = 0x00U;
  }

  return distance_sum;
}

