## Makefile

INCLUDES := ./includes ./drivers
OOCDCONF := ./openocd.cfg

CC := arm-none-eabi-gcc
LINKER_SCRIPT := link.ld

TEST ?= practica4
APP_SRCS := $(TEST).c
TARGET := $(TEST).elf
MAP := $(TEST).map

COMMON_SRCS  := startup.c crc8.c crc8_asm.s \
$(wildcard includes/*.c) \
$(wildcard drivers/*.c)

SRCS := $(APP_SRCS) $(COMMON_SRCS)
OBJS := $(SRCS:.c=.o)
OBJS := $(OBJS:.s=.o)

CFLAGS  := -DCPU_MKL46Z256VLL4 -DSDK_I2C_BASED_COMPONENT_USED -I. $(addprefix -I,$(INCLUDES)) -O2 -Wall -mthumb -mcpu=cortex-m0plus
LDFLAGS := -O2 -mthumb -mcpu=cortex-m0plus --specs=nosys.specs -Wl,--gc-sections,-T$(LINKER_SCRIPT)

.PHONY: all flash clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -Wl,-Map,$(MAP) $^ -o $@ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CC) $(CFLAGS) -c $< -o $@

flash: $(TARGET)
	\openocd -f $(OOCDCONF) -c "program $< verify reset exit"

clean:
	\rm -f $(wildcard *.o includes/*.o drivers/*.o) $(wildcard *.elf) $(wildcard *.map)
