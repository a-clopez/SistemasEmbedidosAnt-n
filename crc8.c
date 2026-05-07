/*
 * crc8.c - implementacións do cálculo de CRC8
 * 
 * Implementación naive: cálculo CRC8 bit a bit sinxelo
 * Polinomio: 0x07 (x^7 + x^2 + x + 1, habitual en CRC8)
 */

#include "crc8.h"

/*
 * Implementación naive de CRC8
 * Procesa cada byte cun bucle simple de desprazamentos e XOR
 * Serve como referencia para comparar coas versións optimizadas.
 */
uint8_t crc8_naive(const uint8_t *data, size_t len)
{
    uint8_t crc = 0x00;  /* Comezar co CRC a 0 */
    
    for (size_t i = 0; i < len; i++)
    {
        uint8_t byte = data[i];
        
        /* Procesar cada bit do byte actual */
        for (uint8_t bit = 0; bit < 8; bit++)
        {
            /* Obter o bit máis significativo do CRC actual */
            uint8_t crc_msb = (crc & 0x80) >> 7;
            
            /* Obter o bit máis significativo do byte actual */
            uint8_t byte_msb = (byte & 0x80) >> 7;
            
            /* Desprazar o CRC á esquerda */
            crc = (crc << 1) & 0xFF;
            
            /* XOR do MSB do CRC co MSB do byte */
            uint8_t xor_bit = crc_msb ^ byte_msb;
            
            /* Se o resultado da XOR é 1, aplicar o polinomio 0x07 */
            if (xor_bit)
            {
                crc ^= 0x07;
            }
            
            /* Desprazar o byte á esquerda para a seguinte iteración */
            byte = (byte << 1) & 0xFF;
        }
    }
    
    return crc;
}

/*
 * A mesma lóxica que crc8_naive, pero forzando unha compilación equivalente a -O0.
 * Úsase para comparar o impacto da optimización do compilador.
 */
__attribute__((noinline, optimize("O0")))
uint8_t crc8_naive_O0(const uint8_t *data, size_t len)
{
    uint8_t crc = 0x00;

    for (size_t i = 0; i < len; i++)
    {
        uint8_t byte = data[i];

        for (uint8_t bit = 0; bit < 8; bit++)
        {
            uint8_t crc_msb = (crc & 0x80) >> 7;
            uint8_t byte_msb = (byte & 0x80) >> 7;
            crc = (crc << 1) & 0xFF;
            uint8_t xor_bit = crc_msb ^ byte_msb;
            if (xor_bit)
            {
                crc ^= 0x07;
            }
            byte = (byte << 1) & 0xFF;
        }
    }

    return crc;
}

/*
 * A mesma lóxica que crc8_naive, pero forzando unha compilación equivalente a -Ofast.
 * Úsase para comparar unha versión máis agresivamente optimizada do compilador.
 */
__attribute__((noinline, optimize("Ofast")))
uint8_t crc8_naive_Ofast(const uint8_t *data, size_t len)
{
    uint8_t crc = 0x00;

    for (size_t i = 0; i < len; i++)
    {
        uint8_t byte = data[i];

        for (uint8_t bit = 0; bit < 8; bit++)
        {
            uint8_t crc_msb = (crc & 0x80) >> 7;
            uint8_t byte_msb = (byte & 0x80) >> 7;
            crc = (crc << 1) & 0xFF;
            uint8_t xor_bit = crc_msb ^ byte_msb;
            if (xor_bit)
            {
                crc ^= 0x07;
            }
            byte = (byte << 1) & 0xFF;
        }
    }

    return crc;
}

/*
 * Implementación optimizada de CRC8 en assembly
 * (definida en crc8_asm.s)
 */
uint8_t crc8_asm(const uint8_t *data, size_t len);
