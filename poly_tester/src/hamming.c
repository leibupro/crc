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
 * Credits:   Thanks to Gurmeet Manku who wrote this article:
 *            http://gurmeet.net/puzzles/fast-bit-counting-routines/
 *            The parallel bitcount function was taken from there
 *            and slightly modified to count ones in uint64_t
 *            integers.
 *
 *
 * Date:      01/2018 
 * 
 */

#include <hamming.h>
#include <stdint.h>
#include <stdio.h>


/* This function is only available on the
 * GNU C Compiler. */
static inline int8_t gcc_builtin_bitcount( uint64_t n );

static inline int8_t parallel_bitcount( uint64_t n );


static int8_t ( *bitcount_func )( uint64_t ) = &gcc_builtin_bitcount;


uint32_t get_hamming_distance( const uint8_t* field_a, 
                               const uint8_t* field_b, uint16_t len )
{
  uint32_t distance_sum = 0U;
  uint16_t i;
  uint8_t  cur_xor;
  uint8_t  cur_sum = 0x00U;

  if( len >= UINT16_MAX )
  {
    ( void )fprintf( stderr, "Invalid number of bytes passed to crc algorithm.\n" );
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


/* Parallel   Count   carries   out    bit   counting   in   a   parallel
   fashion.   Consider   n   after    the   first   line   has   finished
   executing. Imagine splitting n into  pairs of bits. Each pair contains
   the <em>number of ones</em> in those two bit positions in the original
   n.  After the second line has finished executing, each nibble contains
   the  <em>number of  ones</em>  in  those four  bits  positions in  the
   original n. Continuing  this for five iterations, the  64 bits contain
   the  number  of ones  among  these  sixty-four  bit positions  in  the
   original n. That is what we wanted to compute. */

#define TWO( c ) ( 0x1UL << ( c ) )
#define MASK( c ) ( ( ( uint64_t )( -1 ) ) / ( TWO( TWO( c ) ) + 1UL ) )
#define COUNT( x, c ) ( ( x ) & MASK( c ) ) + ( ( ( x ) >> ( TWO( c ) ) ) & MASK( c ) )

static inline int8_t parallel_bitcount( uint64_t n )
{
  n = COUNT( n, 0 );
  n = COUNT( n, 1 );
  n = COUNT( n, 2 );
  n = COUNT( n, 3 );
  n = COUNT( n, 4 );
  n = COUNT( n, 5 );
  return ( int8_t )n;
}


static inline int8_t gcc_builtin_bitcount( uint64_t n )
{
  return ( int8_t )__builtin_popcountl( n );
}


/* works only with two 64 bit integer values as input */
uint8_t get_hamming_distance_opt( uint64_t field_a, uint64_t field_b )
{
  uint64_t cur_xor;
  cur_xor = field_a ^ field_b;
  return ( uint8_t )bitcount_func( cur_xor );
}

