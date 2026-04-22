// Práctica 4

#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// Board includes
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "lcd.h"

// Configuración da práctica
#define QUEUE_LENGTH         10
#define PRODUCER_DELAY_MS    1500
#define CONSUMER_DELAY_MS    2000
#define CONTROL_DELAY_MS     50
#define MAX_TASKS            5

// Tipo de dato para a cola
typedef struct {
    uint32_t id;
    uint32_t data;
    TickType_t timestamp;
} Message_t;

// Handle da cola
static QueueHandle_t xQueue;

// Estado global do sistema
static uint8_t producers_target = 1;
static uint8_t consumers_target = 1;

// Prototipos de tarefas e funcións
static void vTaskProducer(void *pvParameters);
static void vTaskConsumer(void *pvParameters);
static void vTaskControl(void *pvParameters);
static void vTaskDisplay(void *pvParameters);
static void init_buttons(void);

// Función principal
int main(void)
{
    // Inicialización do hardware
    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    init_buttons();
    LCD_Init();

    PRINTF("\r\n=== PRACTICA 4 ===\r\n");
    PRINTF("Escalado dinamico 0..5\r\n");
    PRINTF("Boton Esquerdo: +Produtores | Boton Dereito: +Consumidores\r\n");

    // Crear cola FIFO
    xQueue = xQueueCreate(QUEUE_LENGTH, sizeof(Message_t));
    if (xQueue == NULL) {
        PRINTF("ERRO: Non se puido crear a cola!\r\n");
        for (;;);
    }

    // Crear tarefas produtoras (0 a 4)
    for (uint32_t i = 0; i < MAX_TASKS; i++) {
        if (xTaskCreate(vTaskProducer, "Produtor", configMINIMAL_STACK_SIZE * 2, (void*)i,
                        tskIDLE_PRIORITY + 1, NULL) != pdPASS) {
            PRINTF("ERRO: O produtor %u non puido ser creado!\r\n", (unsigned int)i);
            for (;;);
        }
    }

    // Crear tarefas consumidoras (0 a 4)
    for (uint32_t i = 0; i < MAX_TASKS; i++) {
        if (xTaskCreate(vTaskConsumer, "Consumidor", configMINIMAL_STACK_SIZE * 2, (void*)i,
                        tskIDLE_PRIORITY + 1, NULL) != pdPASS) {
            PRINTF("ERRO: O consumidor %u non puido ser creado!\r\n", (unsigned int)i);
            for (;;);
        }
    }

    // Crear tarefa de control para botóns
    if (xTaskCreate(vTaskControl, "Control", configMINIMAL_STACK_SIZE * 2, NULL,
                    tskIDLE_PRIORITY + 2, NULL) != pdPASS) {
        PRINTF("ERRO: A tarefa de control non puido ser creada!\r\n");
        for (;;);
    }

    // Crear tarefa de actualización do LCD
    if (xTaskCreate(vTaskDisplay, "Display", configMINIMAL_STACK_SIZE * 2, NULL,
                    tskIDLE_PRIORITY + 1, NULL) != pdPASS) {
        PRINTF("ERRO: A tarefa display non puido ser creada!\r\n");
        for (;;);
    }

    // Iniciar scheduler
    PRINTF("Iniciando scheduler RTOS...\r\n");
    vTaskStartScheduler();

    // Non debería chegar aquí
    PRINTF("ERRO: Scheduler terminou!\r\n");
    for (;;);
}


static void init_buttons(void)
{
    CLOCK_EnableClock(kCLOCK_PortC);

    // Configurar mux para GPIO
    PORT_SetPinMux(PORTC, 3U, kPORT_MuxAsGpio);
    PORT_SetPinMux(PORTC, 12U, kPORT_MuxAsGpio);

    // Habilitar pull-ups internas
    PORTC->PCR[3] |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
    PORTC->PCR[12] |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;

    // Configurar como entradas
    gpio_pin_config_t sw_config = {
        kGPIO_DigitalInput,
        0,
    };
    GPIO_PinInit(GPIOC, 3U, &sw_config);
    GPIO_PinInit(GPIOC, 12U, &sw_config);
}

// Tarefa de control para os botóns
static void vTaskControl(void *pvParameters)
{
    uint8_t sw1_prev = 1;
    uint8_t sw3_prev = 1;
    uint8_t sw1_curr, sw3_curr;

    for (;;) {
        sw1_curr = GPIO_PinRead(GPIOC, 3U);
        sw3_curr = GPIO_PinRead(GPIOC, 12U);

        // Detectar flanco de baixada SW1 -> +1 Produtor
        if (sw1_prev == 1 && sw1_curr == 0) {
            producers_target = (producers_target + 1) % (MAX_TASKS + 1);
            PRINTF("=> Boton SW1: Produtores=%d, Consumidores=%d\r\n", producers_target, consumers_target);
        }
        // Detectar flanco de baixada SW3 -> +1 Consumidor
        if (sw3_prev == 1 && sw3_curr == 0) {
            consumers_target = (consumers_target + 1) % (MAX_TASKS + 1);
            PRINTF("=> Boton SW3: Produtores=%d, Consumidores=%d\r\n", producers_target, consumers_target);
        }

        sw1_prev = sw1_curr;
        sw3_prev = sw3_curr;
        vTaskDelay(pdMS_TO_TICKS(CONTROL_DELAY_MS));
    }
}

// Tarefa Produtora
static void vTaskProducer(void *pvParameters)
{
    uint32_t taskId = (uint32_t)pvParameters;
    static uint32_t messageId = 0;
    Message_t message;
    BaseType_t xStatus;

    for (;;) {
        // Se este produtor non debe estar activo segundo o target, esperar un tempo e re-avaliar.
        if (taskId >= producers_target) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        // Xerar datos arbitrarios
        message.id = messageId++;
        message.data = rand() % 1000;  // Número aleatorio 0-999
        message.timestamp = xTaskGetTickCount();

        // Intentar enviar á cola
        xStatus = xQueueSend(xQueue, &message, pdMS_TO_TICKS(100));

        if (xStatus == pdPASS) {
            PRINTF("Produtor[%u]: Enviado ID=%u, Data=%u, Tick=%u\r\n",
                   (unsigned int)taskId, message.id, message.data, message.timestamp);
        } else {
            PRINTF("Produtor[%u]: Cola chea! Non se puido enviar ID=%u\r\n", (unsigned int)taskId, message.id);
        }

        // Delay antes do seguinte envío
        vTaskDelay(pdMS_TO_TICKS(PRODUCER_DELAY_MS));
    }
}

// Tarefa Consumidora
static void vTaskConsumer(void *pvParameters)
{
    uint32_t taskId = (uint32_t)pvParameters;
    Message_t receivedMessage;
    BaseType_t xStatus;
    UBaseType_t uxQueueLength;

    for (;;) {
        // Se este consumidor non debe estar activo segundo o target, esperar un tempo e re-avaliar.
        if (taskId >= consumers_target) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        // Intentar recibir da cola
        xStatus = xQueueReceive(xQueue, &receivedMessage, pdMS_TO_TICKS(100));

        if (xStatus == pdPASS) {
            // Procesar mensaxe
            TickType_t processingTime = xTaskGetTickCount() - receivedMessage.timestamp;

            PRINTF("Consumidor[%u]: Recibido ID=%u, Data=%u, Delay=%ums\r\n",
                   (unsigned int)taskId, receivedMessage.id, receivedMessage.data, processingTime);

            // Simular procesamento
            vTaskDelay(pdMS_TO_TICKS(CONSUMER_DELAY_MS));

            // Mostrar estado da cola
            uxQueueLength = uxQueueMessagesWaiting(xQueue);
            PRINTF("Consumidor[%u]: Cola ten %u elementos pendentes\r\n", (unsigned int)taskId, (unsigned int)uxQueueLength);

        } else {
            // Cola baleira
        }
    }
}
// Tarefa Display LCD
static void vTaskDisplay(void *pvParameters)
{
    UBaseType_t pending;
    
    for (;;) {
        // Obter o numero de elementos pendentes na cola
        pending = uxQueueMessagesWaiting(xQueue);
        
        // Actualizar os valores no LCD
        LCD_DisplayValues((uint8_t)pending, producers_target, consumers_target);
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
