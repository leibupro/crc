/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <pl@vqe.ch> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   P. Leibundgut
 * ----------------------------------------------------------------------------
 *
 * File:      util.h
 * 
 *
 * Purpose:   Helper module holds some generic
 *            functions to do this and that.
 *
 * 
 * Remarks:   -
 *
 *
 * Date:      01/2018 
 * 
 */

#ifndef __UTIL_H_
#define __UTIL_H_

#include <stdint.h>
#include <sys/types.h>

inline void reflect_bits_8 ( uint8_t*  field, uint32_t n );
inline void reflect_bits_16( uint16_t* field, uint32_t n );
inline void reflect_bits_32( uint32_t* field, uint32_t n );
inline void reflect_bits_64( uint64_t* field, uint32_t n );

void check_reflect( uint8_t* buf, uint32_t n, uint8_t reflect );

long try_strtol( char* str );
int32_t walk_file( uint8_t** buf, ssize_t buf_len, 
                   const char* file, uint8_t* more_fragments );

#endif /* __UTIL_H_ */

