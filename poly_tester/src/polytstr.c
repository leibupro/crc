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
#include <hamming.h>

#include <pthread.h>
#include <assert.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NUM_THREADS     2U
#define NUM_INPUT_BYTES 6U

/* 2^48 Possibilities with 48 bit */
/* #define NUM_INPUTS      281474976710656UL */
#define NUM_INPUTS      0x100000000UL

typedef union
{
  uint64_t u_64;
  uint8_t  u_8_arr[ NUM_INPUT_BYTES ];
}
crc_input_t;

typedef struct
{
  uint64_t start;
  uint64_t end;
}
thread_params_t;

typedef struct
{
  uint64_t checksum_counts[ 0x10000U ];
  crc_input_t last_inputs[ 0x10000U ];
}
crc_16_results_t;


/* module local variables of this module. */

static void ( *crc_16_algorithm_func )( uint8_t*, const uint32_t, 
                                        const crc_param_t*, 
                                        uint16_t*, uint8_t ) = NULL;

/* static crc_param_t crc_params = CRC_16_CCITT_FALSE; */
static crc_param_t crc_params = CRC_16_KERMIT;

static crc_16_results_t results_16 = { { 0x0000000000000000UL }, 
                                       { { 0x0000000000000000UL } } };

static pthread_mutex_t results_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t hamming_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_t threads[ NUM_THREADS ];

static thread_params_t thread_params[ NUM_THREADS ];

static uint64_t hamming_dists[ 52 ] = { 0UL };

/* *~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~ */

/* local functions of this module. */

static void test_hamming_meter( void );
static void init( void );
static void deinit( void );
static void create_thread_params( uint8_t num_threads, 
                                  uint64_t num_inputs );

static void* crc_16_worker( void* arg );
static void create_threads( uint32_t* tids );
static void join_threads( void );

/* *~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~ */


static void* crc_16_worker( void* arg )
{
  uint32_t tid;
  crc_input_t crc_input;
  crc_input_t crc_input_loop;
  uint16_t crc16 = 0x0000U;
  uint32_t hamming_dist;
  crc_input_t temp_last;
  uint8_t add_hamming_dist = 0x00U;

  tid = *( ( uint32_t* )arg );

  for( crc_input.u_64 = thread_params[ tid ].start; 
       crc_input.u_64 < thread_params[ tid ].end; 
       crc_input.u_64++ )
  {
    crc_input_loop = crc_input;
    crc_16_algorithm_func( &crc_input_loop.u_8_arr[ 0U ], 
                           ( const uint32_t )NUM_INPUT_BYTES,
                           ( const crc_param_t* )&crc_params, &crc16,
                           0x00U );

    ( void )pthread_mutex_lock( &results_lock );
    /*
     * The hamming distance should only be recorded if
     * the current crc checksum was already calculated
     * at least once.
     **/
    if( results_16.checksum_counts[ crc16 ] )
    {
      add_hamming_dist = 0xFFU;
      temp_last = results_16.last_inputs[ crc16 ];
    }
    results_16.checksum_counts[ crc16 ]++;
    results_16.last_inputs[ crc16 ] = crc_input;
    ( void )pthread_mutex_unlock( &results_lock );
    
    if( add_hamming_dist )
    {
      hamming_dist = get_hamming_distance( 
                       ( const uint8_t* )&crc_input.u_8_arr[ 0U ], 
                       ( const uint8_t* )&temp_last.u_8_arr[ 0U ], 
                       ( uint16_t )NUM_INPUT_BYTES );

      ( void )pthread_mutex_lock( &hamming_lock );
      hamming_dists[ hamming_dist ]++;
      ( void )pthread_mutex_unlock( &hamming_lock );
    }

    add_hamming_dist = 0x00U;
    crc16 = 0x0000U;
  }
  
  ( void )fprintf( stdout, "Hello from thread %d\n", tid );
  ( void )fflush( stdout );

  return NULL;
}


static void init( void )
{
  create_thread_params( NUM_THREADS, NUM_INPUTS );

  if( ( pthread_mutex_init( &results_lock, NULL ) |
        pthread_mutex_init( &hamming_lock, NULL ) ) )
  {
    ( void )fprintf( stderr, "mutex init failed, exiting ...\n");
    ( void )fflush( stderr );
    exit( EXIT_FAILURE );
  }

  /* function from the crc library gets assigned here */
  crc_16_algorithm_func = &crc16_algorithm;
}


static void deinit( void )
{
  if( ( pthread_mutex_destroy( &results_lock ) | 
        pthread_mutex_destroy( &hamming_lock ) ) )
  {
    ( void )fprintf( stderr, "mutex destroy failed, exiting ...\n" );
    ( void )fflush( stderr );
    exit( EXIT_FAILURE );
  }
}


static void create_thread_params( uint8_t num_threads, 
                                  uint64_t num_inputs )
{
  uint64_t part_size = 0UL;
  uint8_t i;
  uint64_t idx_walk = 0UL;

  if( num_inputs % num_threads )
  {
    ( void )fprintf( stderr, "Num threads must be a proper divisor of num inputs.\n" );
    ( void )fflush( stderr );
    exit( EXIT_FAILURE );
  }

  if( num_threads != NUM_THREADS )
  {
    ( void )fprintf( stderr, "Wrong num threads parameter.\n" );
    ( void )fflush( stderr );
    exit( EXIT_FAILURE );
  }

  part_size = ( uint64_t )( num_inputs / num_threads );

  for( i = 0x00U; i < num_threads; i++ )
  {
    thread_params[ i ].start = idx_walk;
    idx_walk += part_size;
    thread_params[ i ].end   = idx_walk;
  }
}


static void create_threads( uint32_t* tids )
{
  uint8_t i;
  int errsv;

  for( i = 0; i < NUM_THREADS; i++ )
  {
    *( tids + i ) = ( uint32_t )i;
    errsv = pthread_create( &threads[ i ], NULL,
                            &crc_16_worker, ( void* )( tids + i ) );

    if( errsv )
    {
      ( void )fprintf( stderr, "can't create thread. reason: [ %s ]\n",
                       strerror( errsv ) );
      ( void )fflush( stderr );
      exit( EXIT_FAILURE );
    }
  }
}


static void join_threads( void )
{
  uint8_t i;
  int errsv;

  for( i = 0; i < NUM_THREADS; i++ )
  {
    errsv = pthread_join( threads[ i ], NULL );
    if( errsv )
    {
      ( void )fprintf( stderr, "can't join thread. reason: [ %s ]\n",
                       strerror( errsv ) );
      ( void )fflush( stderr );
      exit( EXIT_FAILURE );
    }
  }
}


static void test_hamming_meter( void )
{
  uint32_t hamming_dist;

  uint8_t field_a[] = { 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U };
  uint8_t field_b[] = { 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU };
  uint8_t field_c[] = { 0xFFU, 0xFFU, 0x00U, 0x77U, 0xFFU, 0xFFU };

  hamming_dist = get_hamming_distance( 
                   ( const uint8_t* )&field_a[ 0U ], 
                   ( const uint8_t* )&field_b[ 0U ], 
                   ( uint16_t )NUM_INPUT_BYTES );
  assert( hamming_dist == 48U );

  hamming_dist = get_hamming_distance( 
                   ( const uint8_t* )&field_a[ 0U ], 
                   ( const uint8_t* )&field_a[ 0U ], 
                   ( uint16_t )NUM_INPUT_BYTES );
  assert( hamming_dist == 0U );
  
  hamming_dist = get_hamming_distance( 
                   ( const uint8_t* )&field_b[ 0U ], 
                   ( const uint8_t* )&field_b[ 0U ], 
                   ( uint16_t )NUM_INPUT_BYTES );
  assert( hamming_dist == 0U );
  
  hamming_dist = get_hamming_distance( 
                   ( const uint8_t* )&field_a[ 0U ], 
                   ( const uint8_t* )&field_c[ 0U ], 
                   ( uint16_t )NUM_INPUT_BYTES );
  assert( hamming_dist == 38U );
  
  hamming_dist = get_hamming_distance( 
                   ( const uint8_t* )&field_b[ 0U ], 
                   ( const uint8_t* )&field_c[ 0U ], 
                   ( uint16_t )NUM_INPUT_BYTES );
  assert( hamming_dist == 10U );
}


int main( void )
{
  uint32_t tids[ NUM_THREADS ];

  test_hamming_meter();

  init();
  create_threads( &tids[ 0 ] );
  join_threads();
  deinit();

  return EXIT_SUCCESS;
}

