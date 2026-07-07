## Makefile

INCLUDES := ./includes
DRIVERS  := ./drivers
OOCDCONF := ./openocd.cfg

CC := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
LINKER_SCRIPT := link.ld

TARGET_LED   := led_blinky.elf
TARGET_HELLO := hello_world.elf
MAP_LED      := led_blinky.map
MAP_HELLO    := hello_world.map

COMMON_SRCS  := startup.c \
				includes/board.c \
				includes/clock_config.c \
				includes/system_MKL46Z4.c \
				includes/pin_mux.c \
				drivers/fsl_common.c \
				drivers/fsl_gpio.c \
				drivers/fsl_clock.c \
				drivers/fsl_debug_console.c \
				drivers/fsl_smc.c \
				drivers/fsl_ftfx_cache.c \
				drivers/fsl_ftfx_controller.c \
				drivers/fsl_ftfx_flash.c \
				drivers/fsl_log.c \
				drivers/fsl_io.c \
				drivers/fsl_uart.c \
				drivers/fsl_lpsci.c \
				drivers/fsl_str.c \
				drivers/fsl_assert.c 
		
LED_SRCS     := led_blinky.c $(COMMON_SRCS)
HELLO_SRCS   := hello_world.c $(COMMON_SRCS)

LED_OBJS     := $(LED_SRCS:.c=.o)
HELLO_OBJS   := $(HELLO_SRCS:.c=.o)

CFLAGS  := -DCPU_MKL46Z256VLL4 -I $(INCLUDES) -I $(DRIVERS) -O2 -Wall -mthumb -mcpu=cortex-m0plus
LDFLAGS := -O2 -mthumb -mcpu=cortex-m0plus --specs=nosys.specs -Wl,--gc-sections,-T$(LINKER_SCRIPT)

.PHONY: all flash_led flash_hello clean cleanall

all: $(TARGET_LED) $(TARGET_HELLO)

$(TARGET_LED): $(LED_OBJS)
	$(CC) $(LDFLAGS) -Wl,-Map,$(MAP_LED) $^ -o $@

$(TARGET_HELLO): $(HELLO_OBJS)
	$(CC) $(LDFLAGS) -Wl,-Map,$(MAP_HELLO) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

flash_led: $(TARGET_LED)
	openocd -f $(OOCDCONF) -c "program $< verify reset exit"

flash_hello: $(TARGET_HELLO)
	openocd -f $(OOCDCONF) -c "program $< verify reset exit"

clean:
	rm -f $(LED_OBJS) $(HELLO_OBJS)

cleanall: clean
	rm -f $(TARGET_LED) $(TARGET_HELLO) $(MAP_LED) $(MAP_HELLO)
