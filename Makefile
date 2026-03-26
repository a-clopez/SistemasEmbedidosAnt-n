INCLUDES := ./includes
DRIVERS  := ./drivers
OOCDCONF := ./openocd.cfg

CC := arm-none-eabi-gcc
LINKER_SCRIPT := link.ld

TARGET := reverse_practica.elf
MAP    := reverse_practica.map

SRCS := reverse_practica.c \
		reverse_int_asm.s \
		startup.c \
		includes/board.c \
		includes/clock_config.c \
		includes/system_MKL46Z4.c \
		includes/pin_mux.c \
		drivers/fsl_common.c \
		drivers/fsl_clock.c \
		drivers/fsl_debug_console.c \
		drivers/fsl_smc.c \
		drivers/fsl_ftfx_cache.c \
		drivers/fsl_io.c \
		drivers/fsl_log.c \
		drivers/fsl_lpsci.c \
		drivers/fsl_uart.c \
		drivers/fsl_str.c

OBJS := $(patsubst %.c,%.o,$(filter %.c,$(SRCS))) \
		$(patsubst %.s,%.o,$(filter %.s,$(SRCS)))

CFLAGS       := -DCPU_MKL46Z256VLL4 -I $(INCLUDES) -I $(DRIVERS) -O2 -Wall -mthumb -mcpu=cortex-m0plus
CFLAGS_OFAST := -DCPU_MKL46Z256VLL4 -I $(INCLUDES) -I $(DRIVERS) -Ofast -Wall -mthumb -mcpu=cortex-m0plus
LDFLAGS      := -O2 -mthumb -mcpu=cortex-m0plus --specs=nosys.specs -Wl,--gc-sections,-T$(LINKER_SCRIPT)

.PHONY: all flash clean cleanall

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -Wl,-Map,$(MAP) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

reverse_practica.o: reverse_practica.c
	$(CC) $(CFLAGS_OFAST) -c $< -o $@

%.o: %.s
	$(CC) -c -mthumb -mcpu=cortex-m0plus $< -o $@

flash: $(TARGET)
	openocd -f $(OOCDCONF) -c "program $< verify reset exit"

clean:
	rm -f $(OBJS)

cleanall: clean
	rm -f $(TARGET) $(MAP)