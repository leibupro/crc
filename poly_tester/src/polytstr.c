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
#include <mtimer.h>
#include <time.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define NUM_THREADS     2U
#define NUM_INPUT_BYTES 6U

/* 2^48 Possibilities with 48 bit */
/* #define NUM_INPUTS      281474976710656UL */
#define NUM_INPUTS      0x100000000UL

#define RESULTS_FILE      "./crc_occurrences.txt"
#define HAMMING_DIST_FILE "./crc_hamming_distances.txt"
#define STRING_BUF        256U

/* dependant of input field lenght */
#define MAX_HAMMING_DIST  ( NUM_INPUT_BYTES * 8U )

/* thread buffer size for calculations 
 * Required memory => sizeof( crc_16_pair_t ) * NUM_THREADS */
#define WORKER_BUF_SIZE   80000000U

#define TIME_ADD          .0f


typedef union
{
  uint64_t u_64;
  uint8_t  u_8_arr[ NUM_INPUT_BYTES ];
}
crc_input_t;

typedef struct
{
  uint64_t        start;
  uint64_t        end;
  struct timespec init_sleep;
}
thread_params_t;

typedef struct
{
  uint64_t checksum_counts[ 0x10000U ];
  crc_input_t last_inputs[ 0x10000U ];
}
crc_16_results_t;

typedef struct
{
  crc_input_t input;
  uint16_t crc16;
}
crc_16_pair_t;


/* module local variables of this module. */

static void ( *crc_16_algorithm_func )( uint8_t*, const uint32_t, 
                                        const crc_param_t*, 
                                        uint16_t*, uint8_t, uint8_t ) = NULL;

static crc_param_t crc_params = CRC_16_CCITT_FALSE;
/* static crc_param_t crc_params = CRC_16_KERMIT; */
/* static crc_param_t crc_params = CRC_16_SHITTY; */

static crc_16_results_t results_16 = { { 0x0000000000000000UL }, 
                                       { { 0x0000000000000000UL } } };

static pthread_mutex_t results_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_t threads[ NUM_THREADS ];

static thread_params_t thread_params[ NUM_THREADS ];

static uint64_t hamming_dists[ 52 ] = { 0UL };

static crc_16_pair_t* thread_bufs[ NUM_THREADS ] = { NULL };

/* *~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~ */

/* local functions of this module. */

static void test_hamming_meter( void );

static void file_dump_stats( void );
static void write_crc_occurrences( void );
static void write_hamming_distances( void );

static void write_number_as_line( uint64_t number, FILE* file );
static void write_number_pair_as_line( uint64_t number_a, uint64_t number_b, 
                                       FILE* file );

static FILE* open_file( const char* path );
static void close_file( FILE* file );

static void init( void );
static void deinit( void );
static void create_thread_params( uint8_t num_threads, 
                                  uint64_t num_inputs );

static void* crc_16_worker( void* arg );
static void create_threads( uint32_t* tids );
static void join_threads( void );

static void synchronize_results( uint32_t tid, uint32_t num_results );

static void calculate_initial_sleep_times( void );
static struct timespec get_timespec_from_double( double time );

/* *~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~ */


static struct timespec get_timespec_from_double( double time )
{
  struct timespec ts;
  time_t seconds;
  long nanoseconds;
  
  seconds     = ( time_t )time;
  nanoseconds = ( ( long )( time * ( double )1000000000U ) ) - 
                ( ( long )seconds * 1000000000U );
  ts.tv_sec  = seconds;
  ts.tv_nsec = nanoseconds;

  return ts;
}


static void calculate_initial_sleep_times( void )
{
  crc_input_t crc_input;
  crc_input_t crc_input_loop;
  uint16_t crc16 = 0x0000U;
  crc_16_pair_t* buf = NULL;
  crc_16_pair_t* cur_pair = NULL;
  ctimer_t ct_crc;
  ctimer_t ct_sync;
  double estimated_time = .0f;
  uint8_t i;
  
  initCTimer( ct_crc, MONOTONIC );
  initCTimer( ct_sync, MONOTONIC );
  
  buf = thread_bufs[ 0U ];
  
  startCTimer( ct_crc );

  for( crc_input.u_64 = 0UL; crc_input.u_64 < WORKER_BUF_SIZE; crc_input.u_64++ )
  {
    crc_input_loop = crc_input;
    crc_16_algorithm_func( &crc_input_loop.u_8_arr[ 0U ], 
                           ( const uint32_t )NUM_INPUT_BYTES,
                           ( const crc_param_t* )&crc_params, &crc16,
                           0xFFU,   /* First call, yes */
                           0x00U ); /* More fragments, no */

    cur_pair = ( buf + crc_input.u_64 );
    
    cur_pair->input.u_64 = crc_input.u_64;
    cur_pair->crc16 = crc16;
    
    crc16 = 0x0000U;
  }

  stopCTimer( ct_crc );
  
  startCTimer( ct_sync );
  synchronize_results( 0U, WORKER_BUF_SIZE );
  stopCTimer( ct_sync );

  /* clear these results again. */
  ( void )memset( ( void* )&results_16, 0x00000000, sizeof( crc_16_results_t ) );
  ( void )memset( ( void* )&hamming_dists[ 0U ], 0x00000000, sizeof( uint64_t ) * 52 );

  estimated_time = getCTime( ct_crc ) + TIME_ADD;

  for( i = 0U; i < NUM_THREADS; i++ )
  {
    thread_params[ i ].init_sleep = get_timespec_from_double( estimated_time * i );
  }

  ( void )fprintf( stdout, "Estimated buffer fill time on one single core:\n" );
  printCTime( ct_crc );

  ( void )fprintf( stdout, "Estimated synchronisatzion time on one single core:\n" );
  printCTime( ct_sync );
}


static void synchronize_results( uint32_t tid, uint32_t num_results )
{
  uint32_t i;
  crc_16_pair_t* buf = NULL;
  crc_16_pair_t* cur_pair = NULL;
  uint8_t add_hamming_dist = 0x00U;
  uint32_t hamming_dist;
  crc_input_t temp_last;

  buf = thread_bufs[ tid ];
  
  ( void )pthread_mutex_lock( &results_lock );

  for( i = 0U; i < num_results; i++ )
  {
    cur_pair = ( buf + i );
    /*
     * The hamming distance should only be recorded if
     * the current crc checksum was already calculated
     * at least once.
     **/
    if( results_16.checksum_counts[ cur_pair->crc16 ] )
    {
      add_hamming_dist = 0xFFU;
      temp_last = results_16.last_inputs[ cur_pair->crc16 ];
    }
    results_16.checksum_counts[ cur_pair->crc16 ]++;
    results_16.last_inputs[ cur_pair->crc16 ].u_64 = cur_pair->input.u_64;

    if( add_hamming_dist )
    {
      hamming_dist = ( uint32_t )get_hamming_distance_opt( 
                                   cur_pair->input.u_64, temp_last.u_64 );

      hamming_dists[ hamming_dist ]++;
    }

    add_hamming_dist = 0x00U;
  }

  ( void )pthread_mutex_unlock( &results_lock );
}


static void* crc_16_worker( void* arg )
{
  uint32_t tid;
  crc_input_t crc_input;
  crc_input_t crc_input_loop;
  uint16_t crc16 = 0x0000U;
  uint32_t t_buf_idx = 0U;
  crc_16_pair_t* buf = NULL;
  crc_16_pair_t* cur_pair = NULL;

  tid = *( ( uint32_t* )arg );
  buf = thread_bufs[ tid ];

  ( void )nanosleep( &thread_params[ tid ].init_sleep, NULL );

  for( crc_input.u_64 = thread_params[ tid ].start; 
       crc_input.u_64 < thread_params[ tid ].end; 
       crc_input.u_64++ )
  {
    crc_input_loop = crc_input;
    crc_16_algorithm_func( &crc_input_loop.u_8_arr[ 0U ], 
                           ( const uint32_t )NUM_INPUT_BYTES,
                           ( const crc_param_t* )&crc_params, &crc16,
                           0xFFU,   /* First call, yes */
                           0x00U ); /* More fragments, no */

    cur_pair = ( buf + t_buf_idx );

    cur_pair->input.u_64 = crc_input.u_64;
    cur_pair->crc16 = crc16;
    t_buf_idx++;

    if( t_buf_idx >= WORKER_BUF_SIZE )
    {
      synchronize_results( tid, t_buf_idx );
      t_buf_idx = 0U;
    }

    crc16 = 0x0000U;
  }

  /* synchronize remaining results if there are any */
  synchronize_results( tid, t_buf_idx );
  
  ( void )fprintf( stdout, "Hello from thread %d\n", tid );
  ( void )fflush( stdout );

  return NULL;
}


static void init( void )
{
  uint8_t i;

  create_thread_params( NUM_THREADS, NUM_INPUTS );

  if( pthread_mutex_init( &results_lock, NULL ) )
  {
    ( void )fprintf( stderr, "mutex init failed, exiting ...\n");
    ( void )fflush( stderr );
    exit( EXIT_FAILURE );
  }

  /* Function from the crc library gets assigned here.
   * We use the lookup based crc calculation due to
   * performance reasons. */
  crc_16_algorithm_func = &crc16_algorithm_lut;
  /* Lookup table has to be initialized. */
  init_lut_crc_16( ( const crc_param_t* )&crc_params );

  /* initialize thread buffers */
  for( i = 0U; i < NUM_THREADS; i++ )
  {
    if( !( thread_bufs[ i ] = 
             ( crc_16_pair_t* )calloc( ( size_t )WORKER_BUF_SIZE, 
                                       sizeof( crc_16_pair_t ) ) ) )
    {
      ( void )fprintf( stderr, "Failed to allocate memory for thread buffers.\n" );
      ( void )fflush( stderr );
      exit( EXIT_FAILURE );
    }
  }
  calculate_initial_sleep_times();
}


static void deinit( void )
{
  uint8_t i;

  if( pthread_mutex_destroy( &results_lock ) ) 
  {
    ( void )fprintf( stderr, "mutex destroy failed, exiting ...\n" );
    ( void )fflush( stderr );
    exit( EXIT_FAILURE );
  }

  /* free thread buffer space */
  for( i = 0U; i < NUM_THREADS; i++ )
  {
    free( ( void* )thread_bufs[ i ] );
    thread_bufs[ i ] = NULL;
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

  uint64_t integer_a = 0x0000000000000000UL;
  uint64_t integer_b = 0x0000FFFFFFFFFFFFUL;
  uint64_t integer_c = 0x0000FFFF7700FFFFUL;

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

  hamming_dist = ( uint32_t )get_hamming_distance_opt( integer_a, integer_b );
  assert( hamming_dist == 48U );
  
  hamming_dist = ( uint32_t )get_hamming_distance_opt( integer_a, integer_a );
  assert( hamming_dist == 0U );
  
  hamming_dist = ( uint32_t )get_hamming_distance_opt( integer_b, integer_b );
  assert( hamming_dist == 0U );
  
  hamming_dist = ( uint32_t )get_hamming_distance_opt( integer_a, integer_c );
  assert( hamming_dist == 38U );
  
  hamming_dist = ( uint32_t )get_hamming_distance_opt( integer_b, integer_c );
  assert( hamming_dist == 10U );
}


static FILE* open_file( const char* path )
{
  FILE* file = NULL;
  file = fopen( path, ( const char* )"w" );
  if( !file )
  {
    ( void )fprintf( stderr, "Error opening file for writing. reason: [ %s ]\n",
                     strerror( errno ) );
    exit( EXIT_FAILURE );
  }
  return file;
}


static void close_file( FILE* file )
{
  if( fflush( file ) )
  {
    ( void )fprintf( stderr, "can't flush file stream. reason: [ %s ]\n",
                     strerror( errno ) );
    exit( EXIT_FAILURE );
  }
  if( fclose( file ) )
  {
    ( void )fprintf( stderr, "can't close file stream. reason: [ %s ]\n",
                     strerror( errno ) );
    exit( EXIT_FAILURE );
  }
}


static void write_number_as_line( uint64_t number, FILE* file )
{
  char text[ STRING_BUF ] = { 0x00 };

  ( void )snprintf( &text[ 0U ], STRING_BUF, "%ld\n", number );
  if( !fputs( ( const char* )text, file ) )
  {
    ( void )fprintf( stderr, "can't write number to file.\n" );
  }
}


static void write_number_pair_as_line( uint64_t number_a, uint64_t number_b, 
                                       FILE* file )
{
  char text[ STRING_BUF ] = { 0x00 };

  ( void )snprintf( &text[ 0U ], STRING_BUF, "%ld;%#014lx\n", number_a, number_b );
  if( !fputs( ( const char* )text, file ) )
  {
    ( void )fprintf( stderr, "can't write number to file.\n" );
  }
}


static void write_crc_occurrences( void )
{
  FILE* results_file = NULL;
  uint32_t i;

  results_file = open_file( ( const char* )RESULTS_FILE );
  
  for( i = 0U; i < 0x10000U; i++ )
  {
    write_number_pair_as_line( results_16.checksum_counts[ i ], 
                               results_16.last_inputs[ i ].u_64, 
                               results_file );
  }

  close_file( results_file );
}


static void write_hamming_distances( void )
{
  FILE* results_file = NULL;
  uint8_t i;

  results_file = open_file( ( const char* )HAMMING_DIST_FILE );
  
  for( i = 0U; i <= MAX_HAMMING_DIST; i++ )
  {
    write_number_as_line( hamming_dists[ i ], results_file );
  }

  close_file( results_file );
}


static void file_dump_stats( void )
{
  write_crc_occurrences();
  write_hamming_distances();
}


int main( void )
{
  uint32_t tids[ NUM_THREADS ];
  ctimer_t ct;
  
  initCTimer( ct, MONOTONIC );
  test_hamming_meter();

  init();

  startCTimer( ct );
  create_threads( &tids[ 0 ] );
  join_threads();
  stopCTimer( ct );
  
  deinit();

  ( void )fprintf( stdout, "\n\nCRC calculation time stats:\n" );
  printCTime( ct );

  file_dump_stats();

  return EXIT_SUCCESS;
}

