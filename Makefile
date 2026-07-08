## Makefile

INCLUDES := ./includes ./FreeRTOS/include ./FreeRTOS/portable/GCC/ARM_CM0
DRIVERS  := ./drivers
FREERTOS := ./FreeRTOS
OOCDCONF := ./openocd.cfg

CC := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
LINKER_SCRIPT := link.ld

TARGET := practica4.elf
MAP    := practica4.map

COMMON_SRCS  := startup.c \
$(wildcard includes/*.c) \
$(wildcard drivers/*.c)

FREERTOS_SRCS := $(wildcard $(FREERTOS)/*.c) \
 $(wildcard $(FREERTOS)/portable/GCC/ARM_CM0/*.c) \
 $(FREERTOS)/portable/MemMang/heap_4.c

SRCS := practica4.c $(COMMON_SRCS) $(FREERTOS_SRCS)
OBJS := $(SRCS:.c=.o)

CFLAGS  := -DCPU_MKL46Z256VLL4 -I. -I./includes -I$(FREERTOS)/include -I$(FREERTOS)/include/private -I$(FREERTOS)/portable/GCC/ARM_CM0 -I./drivers -O2 -Wall -mthumb -mcpu=cortex-m0plus
LDFLAGS := -O2 -mthumb -mcpu=cortex-m0plus --specs=nosys.specs -Wl,--gc-sections,-T$(LINKER_SCRIPT)

.PHONY: all flash clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -Wl,-Map,$(MAP) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CC) $(CFLAGS) -c $< -o $@

flash: $(TARGET)
	\openocd -f $(OOCDCONF) -c "program $< verify reset exit"

clean:
	\rm -f $(OBJS) $(TARGET) $(MAP)
