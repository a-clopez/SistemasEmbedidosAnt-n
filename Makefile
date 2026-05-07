## Makefile

INCLUDES := ./includes ./drivers
OOCDCONF := ./openocd.cfg

CC := arm-none-eabi-gcc
LINKER_SCRIPT := link.ld

APP_SRCS := practica4.c
TARGET := practica4.elf
MAP := practica4.map

COMMON_SRCS  := startup.c crc8.c crc8_asm.s \
$(wildcard includes/*.c) \
$(wildcard drivers/*.c)

SRCS := $(APP_SRCS) $(COMMON_SRCS)
SRCS_C := $(filter %.c,$(SRCS))
SRCS_S := $(filter %.s,$(SRCS))
OBJS := $(SRCS_C:.c=.o) $(SRCS_S:.s=.o)

CFLAGS  := -DCPU_MKL46Z256VLL4 -I. $(addprefix -I,$(INCLUDES)) -O2 -Wall -mthumb -mcpu=cortex-m0plus
LDFLAGS := -O2 -mthumb -mcpu=cortex-m0plus --specs=nosys.specs -Wl,--gc-sections,-T$(LINKER_SCRIPT)

.PHONY: all flash clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -Wl,-Map,$(MAP) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CC) $(CFLAGS) -c $< -o $@

flash: $(TARGET)
	\openocd -f $(OOCDCONF) -c "program $< verify reset exit"

clean:
	\rm -f $(wildcard *.o includes/*.o drivers/*.o) crc8_asm.o $(TARGET) $(MAP) practica4.elf practica4.map
