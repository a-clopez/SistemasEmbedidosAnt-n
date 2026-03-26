#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_port.h"
#include "board.h"
#include "pin_mux.h"

static inline void init_debug_uart_pins(void)
{
    CLOCK_EnableClock(kCLOCK_PortA);

    PORT_SetPinMux(BOARD_DEBUG_UART_RX_PORT, BOARD_DEBUG_UART_RX_PIN, kPORT_MuxAlt2);
    PORT_SetPinMux(BOARD_DEBUG_UART_TX_PORT, BOARD_DEBUG_UART_TX_PIN, kPORT_MuxAlt2);

    SIM->SOPT5 = (SIM->SOPT5 & ~(SIM_SOPT5_UART0RXSRC_MASK | SIM_SOPT5_UART0TXSRC_MASK)) |
                 SIM_SOPT5_UART0RXSRC(SOPT5_UART0RXSRC_UART_RX) |
                 SIM_SOPT5_UART0TXSRC(SOPT5_UART0TXSRC_UART_TX);
}

unsigned int reverse_int_c(unsigned int in)
{
    unsigned int out = 0U;

    for (unsigned int i = 0U; i < 32U; i++)
    {
        out = out << 1;
        out |= in & 1U;
        in = in >> 1;
    }

    return out;
}

__attribute__((naked)) unsigned int reverse_int_inline_asm(unsigned int in)
{
    (void)in;
    __asm volatile(
        ".syntax unified\n"
        ".thumb\n"
        "ldr r1, =0x55555555\n"
        "lsrs r2, r0, #1\n"
        "ands r2, r1\n"
        "ands r0, r1\n"
        "lsls r0, r0, #1\n"
        "orrs r0, r2\n"

        "ldr r1, =0x33333333\n"
        "lsrs r2, r0, #2\n"
        "ands r2, r1\n"
        "ands r0, r1\n"
        "lsls r0, r0, #2\n"
        "orrs r0, r2\n"

        "ldr r1, =0x0F0F0F0F\n"
        "lsrs r2, r0, #4\n"
        "ands r2, r1\n"
        "ands r0, r1\n"
        "lsls r0, r0, #4\n"
        "orrs r0, r2\n"

        "ldr r1, =0x00FF00FF\n"
        "lsrs r2, r0, #8\n"
        "ands r2, r1\n"
        "ands r0, r1\n"
        "lsls r0, r0, #8\n"
        "orrs r0, r2\n"

        "lsrs r2, r0, #16\n"
        "lsls r0, r0, #16\n"
        "orrs r0, r2\n"
        "bx lr\n");
}

extern unsigned int reverse_int_file_asm(unsigned int in);

static inline void systick_cycle_counter_init(void)
{
    SysTick->CTRL = 0U;
    SysTick->LOAD = 0x00FFFFFFU;
    SysTick->VAL = 0U;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
}

static inline uint32_t timestamp(void)
{
    return SysTick->VAL;
}

static inline uint32_t elapsed_cycles(uint32_t start, uint32_t end)
{
    return (start - end) & 0x00FFFFFFU;
}

int main(void)
{
    const uint32_t input = 0x12345678U;
    uint32_t start;
    uint32_t end;
    uint32_t cycles_c;
    uint32_t cycles_inline;
    uint32_t cycles_file;
    uint32_t out_c;
    uint32_t out_inline;
    uint32_t out_file;

    BOARD_InitPins();
    init_debug_uart_pins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    systick_cycle_counter_init();

    start = timestamp();
    out_c = reverse_int_c(input);
    end = timestamp();
    cycles_c = elapsed_cycles(start, end);

    start = timestamp();
    out_inline = reverse_int_inline_asm(input);
    end = timestamp();
    cycles_inline = elapsed_cycles(start, end);

    start = timestamp();
    out_file = reverse_int_file_asm(input);
    end = timestamp();
    cycles_file = elapsed_cycles(start, end);

    PRINTF("input=0x%08x\r\n", input);
    PRINTF("C       out=0x%08x cycles=%u\r\n", out_c, cycles_c);
    PRINTF("Inline  out=0x%08x cycles=%u\r\n", out_inline, cycles_inline);
    PRINTF("ASM .s  out=0x%08x cycles=%u\r\n", out_file, cycles_file);
}
