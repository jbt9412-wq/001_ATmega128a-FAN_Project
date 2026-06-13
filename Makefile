MCU = atmega128a
F_CPU = 16000000UL
TARGET = fan_project
SRCS = main.c 02_fan_register.c 03_fan_uart.c 04_fan_LED_FND.c 05_fan_motor.c 06_fan_button.c
OBJS = $(SRCS:.c=.o)

CC = avr-gcc
OBJCOPY = avr-objcopy
CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -O2 -Wall

# ISP 프로그래머 설정
PROGRAMMER = usbasp
PORT = usb

all: $(TARGET).hex

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

flash: $(TARGET).hex
	avrdude -c $(PROGRAMMER) -p $(MCU) -P $(PORT) -U flash:w:$<:a

clean:
	rm -f *.o *.elf *.hex