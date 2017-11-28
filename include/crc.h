/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <pl@vqe.ch> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   P. Leibundgut
 * ----------------------------------------------------------------------------
 *
 * File:      crc.h
 * 
 *
 * Purpose:   This header holds all the
 *            and data structures which are
 *            needed to calculate CRC 
 *            checksums and can be included
 *            in modules to perform CRC 
 *            calculations bitwyse 
 *            or bytewise.
 *
 * 
 * Remarks:   -
 *
 *
 * Date:      11/2017 
 * 
 */

#ifndef __CRC_H_
#define __CRC_H_

#include <stdint.h>

#ifndef FILE_BUF_SIZE
#define FILE_BUF_SIZE ( 64UL * 1024UL * 1024UL )
#endif

typedef union data
{
  uint8_t*  u_8;
  uint16_t* u_16;
  uint32_t* u_32;
  uint64_t* u_64;

} data_u;


typedef struct crc_param
{
  uint8_t degree;
  data_u  coeff;
  data_u  initial_xor;
  data_u  final_xor;
  uint8_t reflect_input;
  uint8_t reflect_remainder;
  
} crc_param_t;

#endif /* __CRC_H_ */
