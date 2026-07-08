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

// Estado global do sistema (volatile: lido por varias tarefas)
static volatile uint8_t producers_target = 1;
static volatile uint8_t consumers_target = 1;

// Flags de botón escritos dende a ISR (flanco de baixada)
static volatile uint8_t g_sw1_pressed = 0;
static volatile uint8_t g_sw3_pressed = 0;

// Prototipos de tarefas e funcións
static void vTaskProducer(void *pvParameters);
static void vTaskConsumer(void *pvParameters);
static void vTaskControl(void *pvParameters);
static void vTaskDisplay(void *pvParameters);
static void init_buttons(void);

// Función principal
int main(void)
{
    /* O startup non chama a SystemInit(); desactivamos o watchdog aquí. */
    SystemInit();

    // Inicialización do hardware
    BOARD_InitPins();
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

    // Interrupción por flanco de baixada nos dous botóns
    PORT_SetPinInterruptConfig(PORTC, 3U, kPORT_InterruptFallingEdge);
    PORT_SetPinInterruptConfig(PORTC, 12U, kPORT_InterruptFallingEdge);
    EnableIRQ(PORTC_PORTD_IRQn);
}

/* ISR para PORTC/PORTD. O startup de Practica_1 chámala PORTDIntHandler. */
void PORTDIntHandler(void)
{
    uint32_t flags = GPIO_PortGetInterruptFlags(GPIOC);

    if (flags & (1U << 3U)) {
        g_sw1_pressed = 1;
    }
    if (flags & (1U << 12U)) {
        g_sw3_pressed = 1;
    }

    GPIO_PortClearInterruptFlags(GPIOC, flags);
#if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
#endif
}

// Tarefa de control para os botóns (lectura de flags postas pola ISR)
static void vTaskControl(void *pvParameters)
{
    for (;;) {
        if (g_sw1_pressed) {
            g_sw1_pressed = 0;
            producers_target = (producers_target + 1) % (MAX_TASKS + 1);
            PRINTF("=> Boton SW1: Produtores=%d, Consumidores=%d\r\n", producers_target, consumers_target);
            /* Anti-rebote simple: ignorar novas pulsacións durante 200 ms. */
            vTaskDelay(pdMS_TO_TICKS(200));
        }

        if (g_sw3_pressed) {
            g_sw3_pressed = 0;
            consumers_target = (consumers_target + 1) % (MAX_TASKS + 1);
            PRINTF("=> Boton SW3: Produtores=%d, Consumidores=%d\r\n", producers_target, consumers_target);
            vTaskDelay(pdMS_TO_TICKS(200));
        }

        vTaskDelay(pdMS_TO_TICKS(CONTROL_DELAY_MS));
    }
}

// Tarefa Produtora
static void vTaskProducer(void *pvParameters)
{
    uint32_t taskId = (uint32_t)pvParameters;
    uint32_t messageId = 0;
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

/* Hook de desbordamento de pila; para aquí se se estoura algúha tarefa. */
void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    taskDISABLE_INTERRUPTS();
    for (;;);
}
