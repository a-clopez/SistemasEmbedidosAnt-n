## Makefile

# Project paths / tools
INCLUDES := ./includes
OOCDCONF := ./openocd.cfg

CC := arm-none-eabi-gcc
LINKER_SCRIPT := link.ld
MAP := main.map
TARGET := main.elf

# Sources / objects
SOURCES := main.c startup.c
OBJS := $(SOURCES:.c=.o)

# Flags
CFLAGS := -I $(INCLUDES) -O2 -Wall -mthumb -mcpu=cortex-m0plus
LDFLAGS := -O2 -mthumb -mcpu=cortex-m0plus --specs=nano.specs -Wl,--gc-sections,-Map,$(MAP),-T$(LINKER_SCRIPT)

.PHONY: all build flash clean cleanall

all: $(TARGET)

build: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

flash: $(TARGET)
	openocd -f $(OOCDCONF) -c "program $< verify reset exit"

clean:
	rm -f $(OBJS)

cleanall: clean
	rm -f $(TARGET) $(MAP)