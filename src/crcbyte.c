/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <pl@vqe.ch> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   P. Leibundgut
 * ----------------------------------------------------------------------------
 *
 * File:      crcbyte.c
 * 
 *
 * Purpose:   This module holds all the
 *            necessary functions to calculate 
 *            CRC checksums bytewise.
 *
 * 
 * Remarks:   - By now CRC32 polynomials are
 *              tested and fully functional.
 *
 *            - The program crunches the input
 *              streams byte by byte.
 *
 *
 * Date:      11/2017 
 * 
 */

/* Polynimial coefficients where the most sgnificant bit is omitted */

/* CRC-3 example from wikipedia */
static uint8_t  poly_3_even     = 0x03U;
static uint8_t  poly_3_even_ix  = 0x00U;
static uint8_t  poly_3_even_fx  = 0x00U;

/* CRC-8-CCITT */
static uint8_t  poly_8_even     = 0x07U;
static uint8_t  poly_8_even_ix  = 0x00U;
static uint8_t  poly_8_even_fx  = 0x00U;

/* CRC-16-CCITT (zero) */
static uint16_t poly_16_even    = 0x1021U;
static uint16_t poly_16_even_ix = 0x0000U;
static uint16_t poly_16_even_fx = 0x0000U;

/* CRC-32 */
static uint32_t poly_32_even    = 0x04C11DB7U;
static uint32_t poly_32_even_ix = 0xFFFFFFFFU;
static uint32_t poly_32_even_fx = 0xFFFFFFFFU;

/* CRC-64-ISO */
static uint64_t poly_64_even    = 0x000000000000001BU;
static uint64_t poly_64_even_ix = 0x0000000000000000U;
static uint64_t poly_64_even_fx = 0x0000000000000000U;

