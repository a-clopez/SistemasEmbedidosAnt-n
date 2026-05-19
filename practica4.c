#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "fsl_debug_console.h"
#include "board.h"
#include "fsl_i2c.h"
#include "fsl_common.h"
#include "fsl_tpm.h"
#include "fsl_slcd.h"
#include "clock_config.h"
#include "pin_mux.h"

#define MMA8451_WHOAMI 0x1AU
#define MMA8451_WHOAMI_REG 0x0DU
#define MMA8451_XYZ_DATA_CFG 0x0EU
#define MMA8451_CTRL_REG1 0x2AU

#define MAG3110_ADDR 0x0EU
#define MAG3110_WHO_AM_I 0xC4U
#define MAG3110_WHO_AM_I_REG 0x07U
#define MAG3110_DR_STATUS 0x00U
#define MAG3110_CTRL_REG1 0x10U
#define MAG3110_CTRL_REG2 0x11U

#define TPM_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)
#define TPM_FREQ_HZ 24000U

static int32_t magOffsetX = 0;
static int32_t magOffsetY = 0;

static void SLCD_InitBoard(void)
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
    config.slcdLowPinEnabled = 0x000e0d80U;
    config.slcdHighPinEnabled = 0x00300160U;
    config.backPlaneLowPin = 0x000c0000U;
    config.backPlaneHighPin = 0x00100100U;
    config.faultConfig = NULL;

    SLCD_Init(LCD, &config);

    SLCD_SetBackPlanePhase(LCD, 40, kSLCD_PhaseAActivate);
    SLCD_SetBackPlanePhase(LCD, 52, kSLCD_PhaseBActivate);
    SLCD_SetBackPlanePhase(LCD, 19, kSLCD_PhaseCActivate);
    SLCD_SetBackPlanePhase(LCD, 18, kSLCD_PhaseDActivate);

    SLCD_StartDisplay(LCD);
}

static void SLCD_ClearAll(void)
{
    uint32_t pins[] = {37, 17, 7, 8, 53, 38, 10, 11};
    for (uint32_t i = 0; i < sizeof(pins) / sizeof(pins[0]); i++)
    {
        SLCD_SetFrontPlaneSegments(LCD, pins[i], 0);
    }
}

static const struct {
    uint8_t segDEGF;
    uint8_t segCBA;
} digitTable[10] = {
    { 0x0B, 0x0E },
    { 0x00, 0x06 },
    { 0x07, 0x0C },
    { 0x05, 0x0E },
    { 0x0C, 0x06 },
    { 0x0D, 0x0A },
    { 0x0F, 0x0A },
    { 0x00, 0x0E },
    { 0x0F, 0x0E },
    { 0x0D, 0x0E },
};

static const uint32_t pinDEGF[] = { 37,  7, 53, 10 };
static const uint32_t pinCBA[]  = { 17,  8, 38, 11 };

static void SLCD_ShowDigit(uint8_t position, uint8_t digit)
{
    if (position > 3 || digit > 9)
        return;

    SLCD_SetFrontPlaneSegments(LCD, pinDEGF[position], digitTable[digit].segDEGF);
    SLCD_SetFrontPlaneSegments(LCD, pinCBA[position],  digitTable[digit].segCBA);
}

static void SLCD_ShowHeading(uint16_t heading)
{
    SLCD_ClearAll();

    if (heading > 360)
        heading = 360;

    uint8_t digits[3];
    digits[0] = heading / 100;
    digits[1] = (heading % 100) / 10;
    digits[2] = heading % 10;

    uint8_t numDigits;
    uint8_t startPos;

    if (heading >= 100)
    {
        numDigits = 3;
        startPos = 0;
    }
    else if (heading >= 10)
    {
        numDigits = 2;
        startPos = 1;
    }
    else
    {
        numDigits = 1;
        startPos = 2;
    }

    for (uint8_t i = 0; i < numDigits; i++)
    {
        SLCD_ShowDigit(startPos + i, digits[2 - (numDigits - 1) + i]);
    }
}

static void TPM_InitLEDs(void)
{
    tpm_config_t tpmInfo;
    tpm_chnl_pwm_signal_param_t tpmParam[2];

    tpmParam[0].chnlNumber = (tpm_chnl_t)BOARD_TPM_X_CHANNEL;
    tpmParam[0].level = kTPM_LowTrue;
    tpmParam[0].dutyCyclePercent = 0U;

    tpmParam[1].chnlNumber = (tpm_chnl_t)BOARD_TPM_Y_CHANNEL;
    tpmParam[1].level = kTPM_LowTrue;
    tpmParam[1].dutyCyclePercent = 0U;

    TPM_GetDefaultConfig(&tpmInfo);
    CLOCK_SetTpmClock(1U);
    TPM_Init(BOARD_TPM_BASEADDR, &tpmInfo);
    TPM_SetupPwm(BOARD_TPM_BASEADDR, tpmParam, 2U, kTPM_EdgeAlignedPwm, TPM_FREQ_HZ, TPM_SOURCE_CLOCK);
    TPM_StartTimer(BOARD_TPM_BASEADDR, kTPM_SystemClock);
}

static void TPM_UpdateLEDs(uint8_t green_duty, uint8_t red_duty)
{
    TPM_UpdatePwmDutycycle(BOARD_TPM_BASEADDR, (tpm_chnl_t)BOARD_TPM_X_CHANNEL, kTPM_EdgeAlignedPwm, green_duty);
    TPM_UpdatePwmDutycycle(BOARD_TPM_BASEADDR, (tpm_chnl_t)BOARD_TPM_Y_CHANNEL, kTPM_EdgeAlignedPwm, red_duty);
}

static void Mag3110_Calibrate(void)
{
    int32_t minX = 32767, maxX = -32768;
    int32_t minY = 32767, maxY = -32768;
    uint8_t readBuff[7];

    PRINTF("\r\n=== Magnetometer Calibration ===\r\n");
    PRINTF("Rotate the board 360 degrees slowly...\r\n");
    PRINTF("Calibrating");

    for (uint32_t i = 0; i < 60; i++)
    {
        if (BOARD_Accel_I2C_Receive(MAG3110_ADDR, MAG3110_DR_STATUS, 1, readBuff, 7) == kStatus_Success)
        {
            int32_t x = (int32_t)((int16_t)((readBuff[1] << 8) | readBuff[2]));
            int32_t y = (int32_t)((int16_t)((readBuff[3] << 8) | readBuff[4]));

            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
            if (y < minY) minY = y;
            if (y > maxY) maxY = y;
        }

        if (i % 10 == 0)
            PRINTF(".");

        for (volatile uint32_t j = 0; j < 200000U; j++)
            __NOP();
    }

    magOffsetX = (minX + maxX) / 2;
    magOffsetY = (minY + maxY) / 2;

    PRINTF("\r\nCalibration complete!\r\n");
    PRINTF("Offset X: %ld, Offset Y: %ld\r\n", (long)magOffsetX, (long)magOffsetY);
    PRINTF("Range X: [%ld, %ld], Range Y: [%ld, %ld]\r\n",
           (long)minX, (long)maxX, (long)minY, (long)maxY);
    PRINTF("=============================\r\n\r\n");
}

static void delay_ms(uint32_t ms)
{
    for (volatile uint32_t i = 0; i < ms * 4000U; i++)
        __NOP();
}

int main(void)
{
    uint8_t accel_addr = BOARD_MMA8451_ADDR;
    uint8_t who_am_i_value = 0x00;
    status_t status;

    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_I2C_ReleaseBus();
    BOARD_I2C_ConfigurePins();
    BOARD_InitLEDPins();
    BOARD_InitDebugConsole();

    PRINTF("\r\n=== Digital Compass ===\r\n");

    BOARD_Accel_I2C_Init();

    status = BOARD_Accel_I2C_Receive(BOARD_MMA8451_ADDR, MMA8451_WHOAMI_REG, 1, &who_am_i_value, 1);
    if (status == kStatus_Success && who_am_i_value == MMA8451_WHOAMI)
    {
        PRINTF("Found MMA8451 at address 0x%02X (WHO_AM_I=0x%02X)\r\n", accel_addr, who_am_i_value);
    }
    else
    {
        PRINTF("ERROR: MMA8451 not found (status=%ld, WHO_AM_I=0x%02X)\r\n", (long)status, who_am_i_value);
        while (1) { }
    }

    who_am_i_value = 0x00;
    status = BOARD_Accel_I2C_Receive(MAG3110_ADDR, MAG3110_WHO_AM_I_REG, 1, &who_am_i_value, 1);
    if (status == kStatus_Success && who_am_i_value == MAG3110_WHO_AM_I)
    {
        PRINTF("Found MAG3110 at address 0x%02X (WHO_AM_I=0x%02X)\r\n", MAG3110_ADDR, who_am_i_value);
    }
    else
    {
        PRINTF("ERROR: MAG3110 not found (status=%ld, WHO_AM_I=0x%02X)\r\n", (long)status, who_am_i_value);
        while (1) { }
    }

    BOARD_Accel_I2C_Send(accel_addr, MMA8451_CTRL_REG1, 1, 0x00U);
    BOARD_Accel_I2C_Send(accel_addr, MMA8451_XYZ_DATA_CFG, 1, 0x01U);
    BOARD_Accel_I2C_Send(accel_addr, MMA8451_CTRL_REG1, 1, 0x0DU);

    BOARD_Accel_I2C_Send(MAG3110_ADDR, MAG3110_CTRL_REG2, 1, 0x80U);
    BOARD_Accel_I2C_Send(MAG3110_ADDR, MAG3110_CTRL_REG1, 1, 0x01U);

    SLCD_InitBoard();
    TPM_InitLEDs();

    Mag3110_Calibrate();

    PRINTF("Sensors configured. Starting compass...\r\n\r\n");

    uint8_t readBuff[7];
    int32_t mx, my, mz;
    char dir = 'N';
    uint8_t green_duty = 0;
    uint8_t red_duty = 0;

    while (1)
    {
        if (BOARD_Accel_I2C_Receive(MAG3110_ADDR, MAG3110_DR_STATUS, 1, readBuff, 7) == kStatus_Success)
        {
            mx = (int32_t)((int16_t)((readBuff[1] << 8) | readBuff[2])) - magOffsetX;
            my = (int32_t)((int16_t)((readBuff[3] << 8) | readBuff[4])) - magOffsetY;
            mz = (int32_t)((int16_t)((readBuff[5] << 8) | readBuff[6]));

            float heading_rad = atan2f((float)my, (float)mx);
            float heading_deg = heading_rad * 180.0f / 3.14159265f;
            if (heading_deg < 0.0f)
                heading_deg += 360.0f;

            uint16_t heading = (uint16_t)(heading_deg + 0.5f);

            if (heading >= 315 || heading < 45)
                dir = 'N';
            else if (heading < 135)
                dir = 'E';
            else if (heading < 225)
                dir = 'S';
            else
                dir = 'W';

            green_duty = (uint8_t)((1.0f + cosf(heading_rad)) / 2.0f * 100.0f);
            red_duty = (uint8_t)((1.0f - cosf(heading_rad)) / 2.0f * 100.0f);

            SLCD_ShowHeading(heading);
            TPM_UpdateLEDs(green_duty, red_duty);

            PRINTF("Heading: %3u°  Dir: %c  |  Raw: x=%6ld  y=%6ld  z=%6ld  |  Green=%3u%%  Red=%3u%%\r\n",
                   heading, dir, (long)(mx + magOffsetX), (long)(my + magOffsetY), (long)mz, green_duty, red_duty);
        }

        delay_ms(200);
    }
}

void SVC_Handler(void) { }
void PendSV_Handler(void) { }
void SysTick_Handler(void) { }
