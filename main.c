#include "includes/MKL46Z4.h"

/* 
 * NOTA SOBRE LEDS: Segundo os esquemáticos [5, 6], PTD5 é o LED Vermello e PTE29 o Verde.
 * O código mantén as túas etiquetas orixinais pero ten en conta esta posible inversión.
 */

// Retardo para un anti-rebote simple
static void short_delay(void) {
    volatile int i;
    for (i = 0; i < 200000; i++);
}

/* Flags marcadas por la ISR y procesadas en main (evitar trabajo pesado en ISR) */
volatile uint8_t sw1_event = 0;
volatile uint8_t sw3_event = 0;

// Inicialización LED Verde (PTD5) [7]
void led_green_init() {
    SIM->COPC = 0x00; // Desactivar Watchdog se é posible [7]
    SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK; // Activar reloxo Porto D [7, 8]
    PORTD->PCR[5] = (PORTD->PCR[5] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(1); // Configurar como GPIO [7, 10]
    GPIOD->PDDR |= (1U << 5); // Configurar como saída [7, 11]
    GPIOD->PSOR = (1U << 5);  // Apagar (lóxica negativa) [7, 12]
}

void led_green_toggle() {
    GPIOD->PTOR = (1U << 5); // Conmutar estado [7, 13]
}

// Inicialización LED Vermello (PTE29) [7]
void led_red_init() {
    SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK; // Activar reloxo Porto E [7, 8]
    PORTE->PCR[29] = (PORTE->PCR[29] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(1); // Configurar como GPIO [7, 10]
    GPIOE->PDDR |= (1U << 29); // Configurar como saída [7, 11]
    GPIOE->PSOR = (1U << 29);  // Apagar (lóxica negativa) [7, 12]
}

void led_red_toggle(void) {
    GPIOE->PTOR = (1U << 29); // Conmutar estado [7, 13]
}

// Configuración de botóns por interrupción: SW1 (PTC3) e SW3 (PTC12) [15]
void switches_init(void) {
    SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK; // Activar reloxo Porto C [8, 16]

    /* 
     * PCRn: 
     * MUX(1) -> GPIO [10]
     * PE e PS -> Pull-up activo (necesario para botóns que pechan a terra) [17, 18]
     * IRQC(0xA) -> Configurar interrupción por flanco de baixada [19, 20]
     */
    PORTC->PCR[3] = (PORTC->PCR[3] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(1) | 
                    PORT_PCR_PE_MASK | PORT_PCR_PS_MASK | PORT_PCR_IRQC(0x0A);

    PORTC->PCR[12] = (PORTC->PCR[12] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(1) | 
                     PORT_PCR_PE_MASK | PORT_PCR_PS_MASK | PORT_PCR_IRQC(0x0A);

    GPIOC->PDDR &= ~((1U << 3) | (1U << 12)); // Configurar como entradas [11, 15]

    /* Configuración no NVIC (Controlador de interrupcións) */
    // Porto C e D comparten a liña de interrupción 31 [23, 24]
    NVIC_ClearPendingIRQ(PORTC_PORTD_IRQn); // Limpar calquera interrupción previa [25]
    NVIC_SetPriority(PORTC_PORTD_IRQn, 0);  // Establecer prioridade (0 a 3) [26, 27]
    NVIC_EnableIRQ(PORTC_PORTD_IRQn);       // Habilitar a IRQ 31 no NVIC [28, 29]
}

/* e
 * Rutina de Servizo de Interrupción (ISR) 
 * O nome debe coincidir co definido na táboa de vectores do startup code [30, 31].
 */
void PORTDIntHandler(void) {
    // Leer flags y marcar eventos; ISR mínima (no delays)
    uint32_t flags = PORTC->ISFR;

    if (flags & (1U << 3)) {
        sw1_event = 1;                 // procesar en main
        PORTC->ISFR = (1U << 3);       // limpiar W1C
    }

    if (flags & (1U << 12)) {
        sw3_event = 1;
        PORTC->ISFR = (1U << 12);
    }
}


int main(void) {
    // Inicialización de periféricos
    led_green_init();
    led_red_init();
    switches_init();

    /* Ensure global interrupts are enabled */
    __enable_irq();

    /* 
     * O bucle principal xa non precisa facer nada.
     * __WFI() pon ao procesador en modo espera ata a seguinte interrupción [35, 36].
     */
    while (1) {
        if (sw1_event) {
            sw1_event = 0;
            /* verificar estado (pull-up: 1 = released, 0 = pressed) */
            if (!(GPIOC->PDIR & (1U << 3))) {
                led_green_toggle();
                short_delay(); /* debounce manejado en main */
            }
        }

        if (sw3_event) {
            sw3_event = 0;
            if (!(GPIOC->PDIR & (1U << 12))) {
                led_red_toggle();
                short_delay();
            }
        }

        __WFI();
    }

    return 0;
}