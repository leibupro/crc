/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <pl@vqe.ch> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   P. Leibundgut
 * ----------------------------------------------------------------------------
 *
 * File:      crcapi.h
 * 
 *
 * Purpose:   This header holds all the
 *            api function prototypes which 
 *            can be used from the crc
 *            library.
 *            
 * 
 * Remarks:   -
 *
 *
 * Date:      01/2018 
 * 
 */

#ifndef __CRC_API_H_
#define __CRC_API_H_

#include <crctypes.h>
#include <stdint.h>

void crc16_algorithm( uint8_t* data, const uint32_t len, 
                      const crc_param_t* crc_params, uint16_t* p_crc,
                      uint8_t first_call,
                      uint8_t more_fragments );

void init_lut_crc_16( const crc_param_t* crc_params );

void crc16_algorithm_lut( uint8_t* data, const uint32_t len,
                          const crc_param_t* crc_params, uint16_t* p_crc,
                          uint8_t first_call,
                          uint8_t more_fragments );

#endif /* __CRC_API_H_ */

