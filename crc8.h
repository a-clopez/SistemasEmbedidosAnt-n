/*
 * crc8.h - CRC8 calculation implementations
 */

#ifndef CRC8_H_
#define CRC8_H_

#include <stdint.h>
#include <stddef.h>

/*
 * CRC8 naive implementation (baseline for comparison)
 * Calculates CRC8 (polynomial 0x07) over a buffer.
 * 
 * @param data    Pointer to data buffer
 * @param len     Length of data in bytes
 * @return        Calculated CRC8 value
 */
uint8_t crc8_naive(const uint8_t *data, size_t len);

/*
 * CRC8 naive implementation compiled with -O0 equivalent for benchmarking.
 */
uint8_t crc8_naive_O0(const uint8_t *data, size_t len);

/*
 * CRC8 naive implementation compiled with -Ofast equivalent for benchmarking.
 */
uint8_t crc8_naive_Ofast(const uint8_t *data, size_t len);

/*
 * CRC8 optimized assembly implementation
 * (To be implemented in crc8_asm.s)
 * 
 * @param data    Pointer to data buffer
 * @param len     Length of data in bytes
 * @return        Calculated CRC8 value
 */
uint8_t crc8_asm(const uint8_t *data, size_t len);

#endif /* CRC8_H_ */
