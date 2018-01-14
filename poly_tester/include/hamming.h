/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <pl@vqe.ch> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   P. Leibundgut
 * ----------------------------------------------------------------------------
 *
 * File:      hamming.h
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

#ifndef __HAMMING_H_
#define __HAMMING_H_

#include <stdint.h>

uint32_t get_hamming_distance( const uint8_t* field_a, 
                               const uint8_t* field_b, uint16_t len );

#endif /* __HAMMING_H_ */

