/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <pl@vqe.ch> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   P. Leibundgut
 * ----------------------------------------------------------------------------
 *
 * File:      crcparam_even.h
 * 
 *
 * Purpose:   This header holds all the
 *            crc parameters for calculating
 *            CRC checksums bytewise.
 *            Polynimial coefficients where 
 *            the most sgnificant bit is omitted.
 *
 * 
 * Remarks:   - Watch the endianness when 
 *              initializing unions in C89 manner.
 *            
 *            - The initial values defined here
 *              are specified for the crc_param_t
 *              located in crctypes.h
 *
 *
 * Date:      01/2018 
 * 
 */

#ifndef __CRCPARAM_EVEN_H_
#define __CRCPARAM_EVEN_H_


/* some CRC 3 values */

#define CRC_3_WIKIPEDIA    {                \
/* polynomial degree */      3U,            \
/* coefficients */           { { 0x03U } }, \
/* initial */                { { 0x00U } }, \
/* final xor */              { { 0x00U } }, \
/* reflect input */          0x00U,         \
/* reflect remainder */      0x00U          \
                           }


/* some CRC 8 values */

#define CRC_8_CCITT        {                \
/* polynomial degree */      8U,            \
/* coefficients */           { { 0x07U } }, \
/* initial */                { { 0x00U } }, \
/* final xor */              { { 0x00U } }, \
/* reflect input */          0x00U,         \
/* reflect remainder */      0x00U          \
                           }


/* some CRC 16 values */

#define CRC_16_CCITT_FALSE {                       \
/* polynomial degree */      16U,                  \
/* coefficients */           { { 0x21U, 0x10U } }, \
/* initial */                { { 0xFFU, 0xFFU } }, \
/* final xor */              { { 0x00U, 0x00U } }, \
/* reflect input */          0x00U,                \
/* reflect remainder */      0x00U                 \
                           }


/* some CRC 32 values */

#define CRC_32             {                                     \
/* polynomial degree */      32U,                                \
/* coefficients */           { { 0xB7U, 0x1DU, 0xC1U, 0x04U } }, \
/* initial */                { { 0xFFU, 0xFFU, 0xFFU, 0xFFU } }, \
/* final xor */              { { 0xFFU, 0xFFU, 0xFFU, 0xFFU } }, \
/* reflect input */          0xFFU,                              \
/* reflect remainder */      0xFFU                               \
                           }


/* some CRC 64 values */

#define CRC_64_ISO         {                                                                 \
/* polynomial degree */      64U,                                                            \
/* coefficients */           { { 0x1BU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U } }, \
/* initial */                { { 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U } }, \
/* final xor */              { { 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U } }, \
/* reflect input */          0x00U,                                                          \
/* reflect remainder */      0x00U                                                           \
                           }



/* Map the polynomial parameters here, 
 * which should be used by the program. */
#define CRC_3_POLY_PARAM  CRC_3_WIKIPEDIA
#define CRC_8_POLY_PARAM  CRC_8_CCITT
#define CRC_16_POLY_PARAM CRC_16_CCITT_FALSE
#define CRC_32_POLY_PARAM CRC_32
#define CRC_64_POLY_PARAM CRC_64_ISO

#endif /* __CRCPARAM_EVEN_H_ */

