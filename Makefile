## Makefile

INCLUDES := ./includes ./drivers
FREERTOS := ./FreeRTOS-LTS/FreeRTOS/FreeRTOS-Kernel
OOCDCONF := ./openocd.cfg

CC := arm-none-eabi-gcc
LINKER_SCRIPT := link.ld

APP ?= practice

COMMON_SRCS  := startup.c crc8.c crc8_asm.s \
$(wildcard includes/*.c) \
$(wildcard drivers/*.c)

FREERTOS_SRCS := $(wildcard $(FREERTOS)/*.c) \
 $(wildcard $(FREERTOS)/portable/GCC/ARM_CM0/*.c) \
 $(FREERTOS)/portable/MemMang/heap_2.c

ifeq ($(APP),example)
APP_SRCS := examples/tpm/timer/tpm_timer.c
TARGET := tpm_timer.elf
MAP := tpm_timer.map
APP_INCLUDES :=
else ifeq ($(APP),practice)
APP_SRCS := practica4.c
TARGET := practica4.elf
MAP := practica4.map
APP_INCLUDES :=
else
$(error Unknown APP value '$(APP)'. Use APP=practice or APP=example)
endif

SRCS := $(APP_SRCS) $(COMMON_SRCS)
SRCS_C := $(filter %.c,$(SRCS))
SRCS_S := $(filter %.s,$(SRCS))
OBJS := $(SRCS_C:.c=.o) $(SRCS_S:.s=.o)

CFLAGS  := -DCPU_MKL46Z256VLL4 -I. $(addprefix -I,$(INCLUDES)) $(APP_INCLUDES) -O2 -Wall -mthumb -mcpu=cortex-m0plus
LDFLAGS := -O2 -mthumb -mcpu=cortex-m0plus --specs=nosys.specs -Wl,--gc-sections,-T$(LINKER_SCRIPT)

.PHONY: all practice example flash clean

all: $(TARGET)

practice:
	$(MAKE) APP=practice all

example:
	$(MAKE) APP=example all

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -Wl,-Map,$(MAP) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CC) $(CFLAGS) -c $< -o $@

flash: $(TARGET)
	\openocd -f $(OOCDCONF) -c "program $< verify reset exit"

clean:
	\rm -f $(wildcard *.o includes/*.o drivers/*.o examples/tpm/timer/*.o) crc8_asm.o $(TARGET) $(MAP) practica4.elf practica4.map tpm_timer.elf tpm_timer.map
