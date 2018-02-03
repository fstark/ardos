# Assemble
```
avra demo.asm
```

# Write to arduino
```
avrdude -p m328p -c arduino -b 28800 -P /dev/ttyACM0 -U flash:w:demo.hex 
-p m328p = model of AVR
-c arduino = model of bootloader
-b 28800 = baud rate (skip if needed)
-P /dev/ttyACM0 = serial device
-U flash:w:demo.hex = update flash memory, write demo.hex file
```

# Dump memory
```
avrdude -p m328p -c arduino -b 28800 -P /dev/ttyACM0 -U flash:r:flash.hex:i
avrdude -p m328p -c arduino -b 28800 -P /dev/ttyACM0 -U signature:r:sig.hex:h
avrdude -p m328p -c arduino -b 28800 -P /dev/ttyACM0 -U eeprom:r:eeprom.hex:i
```

# Compiling C
```
avr-gcc -g -Os -mmcu=atmega328p -c led.c
avr-gcc -g -mmcu=atmega328p -o led.elf led.o
avr-objcopy -j .text -j .data -O ihex led.elf led.hex
avrdude -p m328p -c arduino -b 28800 -P /dev/ttyACM0 -U flash:w:led.hex 
```

# Connect to serial terminal (exit with "ctrl-d \")
```
screen /dev/ttyACM0 115200
```
