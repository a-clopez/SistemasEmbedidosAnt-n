#include "lcd.h"
#include "fsl_slcd.h"
#include "fsl_port.h"

// SLCD pins for KL46Z
#define LCD_PIN_D1_A 37
#define LCD_PIN_D1_B 17
#define LCD_PIN_D2_A 7
#define LCD_PIN_D2_B 8
#define LCD_PIN_D3_A 53
#define LCD_PIN_D3_B 38
#define LCD_PIN_D4_A 10
#define LCD_PIN_D4_B 11

const uint8_t s_lcd_pins[4][2] = {
    {LCD_PIN_D1_A, LCD_PIN_D1_B},
    {LCD_PIN_D2_A, LCD_PIN_D2_B},
    {LCD_PIN_D3_A, LCD_PIN_D3_B},
    {LCD_PIN_D4_A, LCD_PIN_D4_B}
};

/*
 * Segment mapping for numbers 0-9 
 * Each digit needs 2 pins. 
 * Pin A handles: COM1(D), COM2(E), COM3(G), COM4(F) -> 1, 2, 4, 8
 * Pin B handles: COM1(DEC), COM2(C), COM3(B), COM4(A) -> 1, 2, 4, 8
 * 
 * Segment bits:
 * Pin A: D=1, E=2, G=4, F=8
 * Pin B: DP=1, C=2, B=4, A=8
 */

const uint8_t lcd_num_pinA[10] = {
    0x0B, /* 0: D, E, F */
    0x00, /* 1: None */
    0x07, /* 2: D, E, G */
    0x05, /* 3: D, G */
    0x0C, /* 4: F, G */
    0x0D, /* 5: D, F, G */
    0x0F, /* 6: D, E, F, G */
    0x00, /* 7: None */
    0x0F, /* 8: D, E, F, G */
    0x0D  /* 9: D, F, G */
};

const uint8_t lcd_num_pinB[10] = {
    0x0E, /* 0: A, B, C */
    0x06, /* 1: B, C */
    0x0C, /* 2: A, B */
    0x0E, /* 3: A, B, C */
    0x06, /* 4: B, C */
    0x0A, /* 5: A, C */
    0x0A, /* 6: A, C */
    0x0E, /* 7: A, B, C */
    0x0E, /* 8: A, B, C */
    0x0E  /* 9: A, B, C */
};

void LCD_Init(void)
{
    slcd_config_t config;
    slcd_clock_config_t clkConfig = {
        kSLCD_AlternateClk1,
        kSLCD_AltClkDivFactor1,
        kSLCD_ClkPrescaler01
    };

    SLCD_GetDefaultConfig(&config);
    config.clkConfig = &clkConfig;
    config.loadAdjust = kSLCD_HighLoadOrSlowestClkSrc;
    config.dutyCycle = kSLCD_1Div4DutyCycle;
    
    /* Configuración por defecto para FRDM-KL46Z */
    config.slcdLowPinEnabled = 0x000e0d80U;  
    config.slcdHighPinEnabled = 0x00300160U; 
    config.backPlaneLowPin = 0x000c0000U;    /* PIN19, PIN18 */
    config.backPlaneHighPin = 0x00100100U;   /* PIN52, PIN40 */
    config.faultConfig = NULL;

    SLCD_Init(LCD, &config);

    SLCD_SetBackPlanePhase(LCD, 40, kSLCD_PhaseAActivate); /* COM1 */
    SLCD_SetBackPlanePhase(LCD, 52, kSLCD_PhaseBActivate); /* COM2 */
    SLCD_SetBackPlanePhase(LCD, 19, kSLCD_PhaseCActivate); /* COM3 */
    SLCD_SetBackPlanePhase(LCD, 18, kSLCD_PhaseDActivate); /* COM4 */

    SLCD_StartDisplay(LCD);
}

static void LCD_SetDigit(uint8_t pos, uint8_t value)
{
    if (pos > 3 || value > 9) return;
    
    SLCD_SetFrontPlaneSegments(LCD, s_lcd_pins[pos][0], lcd_num_pinA[value]);
    SLCD_SetFrontPlaneSegments(LCD, s_lcd_pins[pos][1], lcd_num_pinB[value]);
}

static void LCD_ClearDigit(uint8_t pos)
{
    if (pos > 3) return;
    SLCD_SetFrontPlaneSegments(LCD, s_lcd_pins[pos][0], 0x00);
    SLCD_SetFrontPlaneSegments(LCD, s_lcd_pins[pos][1], 0x00);
}

void LCD_DisplayValues(uint8_t pending, uint8_t producers, uint8_t consumers)
{
    /* Díxitos 1 e 2 para 'pending' (00 a 10) */
    if (pending > 9) {
        LCD_SetDigit(0, pending / 10);
        LCD_SetDigit(1, pending % 10);
    } else {
        LCD_ClearDigit(0);
        LCD_SetDigit(1, pending);
    }
    
    /* Díxito 3 para produtores activos (0 a 5) */
    LCD_SetDigit(2, producers);
    
    /* Díxito 4 para consumidores activos (0 a 5) */
    LCD_SetDigit(3, consumers);
}
