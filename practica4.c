#include <stdint.h>
#include <stddef.h>
#include "board.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
#include "fsl_tpm.h"
#include "fsl_common.h"
#include "crc8.h"

#define BOARD_TPM TPM0
#define TPM_SOURCE_CLOCK (CLOCK_GetFreq(kCLOCK_PllFllSelClk)/4)
#ifndef TPM_PRESCALER
#define TPM_PRESCALER kTPM_Prescale_Divide_4
#endif

int main(void)
{
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    tpm_config_t tpmInfo;
    CLOCK_SetTpmClock(1U);
    TPM_GetDefaultConfig(&tpmInfo);
    tpmInfo.prescale = TPM_PRESCALER;
    TPM_Init(BOARD_TPM, &tpmInfo);
    TPM_SetTimerPeriod(BOARD_TPM, 0xFFFFU);
    TPM_StartTimer(BOARD_TPM, kTPM_SystemClock);

    const size_t DATA_LEN = 256;
    uint8_t data[DATA_LEN];
    for (size_t i = 0; i < DATA_LEN; ++i) data[i] = (uint8_t)i;
    const int ITERS = 200;

    // Calcular os valores devoltos polas funcións crc
    uint8_t crc_o0 = crc8_naive_O0(data, DATA_LEN);
    uint8_t crc_ofast = crc8_naive_Ofast(data, DATA_LEN);
    uint8_t crc_asm_val = crc8_asm(data, DATA_LEN);
    
    PRINTF("\r\nCRC values:\r\n");
    PRINTF("O0:    %u\r\n", (unsigned int)crc_o0);
    PRINTF("Ofast: %u\r\n", (unsigned int)crc_ofast);
    PRINTF("ASM:   %u\r\n\r\n", (unsigned int)crc_asm_val);

    uint32_t total_ticks_naive_o0 = 0;
    for (int i = 0; i < ITERS; ++i) {
        uint32_t start = TPM_GetCurrentTimerCount(BOARD_TPM);
        volatile uint8_t r = crc8_naive_O0(data, DATA_LEN);
        (void)r;
        uint32_t end = TPM_GetCurrentTimerCount(BOARD_TPM);
        uint32_t mod = BOARD_TPM->MOD;
        uint32_t ticks = (end >= start) ? (end - start) : (mod + 1U - start + end);
        total_ticks_naive_o0 += ticks;
    }

    uint32_t total_ticks_naive_ofast = 0;
    for (int i = 0; i < ITERS; ++i) {
        uint32_t start = TPM_GetCurrentTimerCount(BOARD_TPM);
        volatile uint8_t r = crc8_naive_Ofast(data, DATA_LEN);
        (void)r;
        uint32_t end = TPM_GetCurrentTimerCount(BOARD_TPM);
        uint32_t mod = BOARD_TPM->MOD;
        uint32_t ticks = (end >= start) ? (end - start) : (mod + 1U - start + end);
        total_ticks_naive_ofast += ticks;
    }

    uint32_t total_ticks_asm = 0;
    for (int i = 0; i < ITERS; ++i) {
        uint32_t start = TPM_GetCurrentTimerCount(BOARD_TPM);
        volatile uint8_t r = crc8_asm(data, DATA_LEN);
        (void)r;
        uint32_t end = TPM_GetCurrentTimerCount(BOARD_TPM);
        uint32_t mod = BOARD_TPM->MOD;
        uint32_t ticks = (end >= start) ? (end - start) : (mod + 1U - start + end);
        total_ticks_asm += ticks;
    }

    uint32_t prescale_div = (1U << (uint32_t)TPM_PRESCALER);
    uint64_t tpm_clk = (uint64_t)TPM_SOURCE_CLOCK / prescale_div;

    uint32_t avg_ticks_naive_o0 = total_ticks_naive_o0 / ITERS;
    uint32_t avg_us_naive_o0 = (uint32_t)COUNT_TO_USEC(avg_ticks_naive_o0, tpm_clk);

    uint32_t avg_ticks_naive_ofast = total_ticks_naive_ofast / ITERS;
    uint32_t avg_us_naive_ofast = (uint32_t)COUNT_TO_USEC(avg_ticks_naive_ofast, tpm_clk);

    uint32_t avg_ticks_asm = total_ticks_asm / ITERS;
    uint32_t avg_us_asm = (uint32_t)COUNT_TO_USEC(avg_ticks_asm, tpm_clk);

    PRINTF("Benchmark (256B, 200x):\r\n");
    PRINTF("O0:    %u ticks, %u us\r\n", (unsigned int)avg_ticks_naive_o0, (unsigned int)avg_us_naive_o0);
    PRINTF("Ofast: %u ticks, %u us\r\n", (unsigned int)avg_ticks_naive_ofast, (unsigned int)avg_us_naive_ofast);
    PRINTF("ASM:   %u ticks, %u us\r\n", (unsigned int)avg_ticks_asm, (unsigned int)avg_us_asm);

    TPM_StopTimer(BOARD_TPM);

    // Done: loop forever
    while (1) { __asm__("wfi"); }
}

/* Para que no startup non salte un erro */
void SVC_Handler(void) { }
void PendSV_Handler(void) { }
void SysTick_Handler(void) { }
