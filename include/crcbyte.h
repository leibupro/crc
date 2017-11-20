/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <pl@vqe.ch> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   P. Leibundgut
 * ----------------------------------------------------------------------------
 *
 * File:      crcbyte.h
 * 
 *
 * Purpose:   This header holds all the
 *            necessary public function prototypes 
 *            to calculate CRC checksums and
 *            can be included in modules to
 *            perform CRC calculations bytewise.
 *
 * 
 * Remarks:   -
 *
 *
 * Date:      11/2017 
 * 
 */

#ifndef __CRCBYTE_H_
#define __CRCBYTE_H_

#include <stdint.h>


void calculate_crc_from_file_bytewise( char* const file, 
                                       uint8_t polynomial_degree );

#endif /* __CRCBYTE_H_ */

