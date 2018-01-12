/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <pl@vqe.ch> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   P. Leibundgut
 * ----------------------------------------------------------------------------
 *
 * File:      crcparam_odd.h
 * 
 *
 * Purpose:   This header holds all the
 *            crc parameters for calculating
 *            CRC checksums bitwise.
 *
 * 
 * Remarks:   - The initial values defined here
 *              are specified for the crc_param_t
 *              located in crctypes.h
 *
 *
 * Date:      01/2018 
 * 
 */

/* CRC polynomial coefficients 
 * OK, these polynomials were taken from this source: 
 * https://en.wikipedia.org/wiki/Polynomial_representations_of_cyclic_redundancy_checks
 * The omission of the high-order bit was not applied here.
 * We have an extra byte for the odd bit.
 *
 * E. g.:
 * CRC32, given polynomial coefficients: 0x04C11DB7
 * Added a 1 for the first bit (x^32): 0x104C11DB7
 * Left shift of seven bit: 0x82608EDB80
 */

#ifndef __CRCPARAM_ODD_H_
#define __CRCPARAM_ODD_H_


/* some CRC 3 values */

#define CRC_3_WIKIPEDIA    {                \
/* polynomial degree */      3U,            \
/* coefficients */           { { 0xB0U } }, \
/* initial */                { { 0x00U } }, \
/* final xor */              { { 0x00U } }, \
/* reflect input */          0x00U,         \
/* reflect remainder */      0x00U          \
                           }


/* some CRC 8 values */

#define CRC_8_CCITT        {                       \
/* polynomial degree */      8U,                   \
/* coefficients */           { { 0x83U, 0x80U } }, \
/* initial */                { { 0x00U } },        \
/* final xor */              { { 0x00U } },        \
/* reflect input */          0x00U,                \
/* reflect remainder */      0x00U                 \
                           }


/* some CRC 16 values */

#define CRC_16_CCITT_FALSE {                              \
/* polynomial degree */      16U,                         \
/* coefficients */           { { 0x88U, 0x10U, 0x80U } }, \
/* initial */                { { 0xFFU, 0xFFU } },        \
/* final xor */              { { 0x00U, 0x00U } },        \
/* reflect input */          0x00U,                       \
/* reflect remainder */      0x00U                        \
                           }

#define CRC_16_KERMIT      {                              \
/* polynomial degree */      16U,                         \
/* coefficients */           { { 0x88U, 0x10U, 0x80U } }, \
/* initial */                { { 0x00U, 0x00U } },        \
/* final xor */              { { 0x00U, 0x00U } },        \
/* reflect input */          0xFFU,                       \
/* reflect remainder */      0xFFU                        \
                           }


/* some CRC 32 values */

#define CRC_32             {                                            \
/* polynomial degree */      32U,                                       \
/* coefficients */           { { 0x82U, 0x60U, 0x8EU, 0xDBU, 0x80U } }, \
/* initial */                { { 0xFFU, 0xFFU, 0xFFU, 0xFFU } },        \
/* final xor */              { { 0xFFU, 0xFFU, 0xFFU, 0xFFU } },        \
/* reflect input */          0xFFU,                                     \
/* reflect remainder */      0xFFU                                      \
                           }


/* some CRC 64 values */

#define CRC_64_ISO         {                                                                        \
/* polynomial degree */      64U,                                                                   \
/* coefficients */           { { 0x80U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x0DU, 0x80U } }, \
/* initial */                { { 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U } },        \
/* final xor */              { { 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U } },        \
/* reflect input */          0x00U,                                                                 \
/* reflect remainder */      0x00U                                                                  \
                           }



/* Map the polynomial parameters here, 
 * which should be used by the program. */
#define CRC_3_POLY_PARAM  CRC_3_WIKIPEDIA
#define CRC_8_POLY_PARAM  CRC_8_CCITT
#define CRC_16_POLY_PARAM CRC_16_KERMIT
#define CRC_32_POLY_PARAM CRC_32
#define CRC_64_POLY_PARAM CRC_64_ISO

#endif /* __CRCPARAM_ODD_H_ */

