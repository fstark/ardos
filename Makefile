# C Sample

ardos: ardos.hex
	avrdude -p m328p -c arduino -P /dev/ttyACM0 -U flash:w:ardos.hex 

ardos.hex: ardos.c
	avr-gcc -fno-toplevel-reorder -Os -mmcu=atmega328p -c ardos.c
	avr-gcc -mmcu=atmega328p -o ardos.elf ardos.o
	avr-objcopy -j .text -j .data -O ihex ardos.elf ardos.hex



cled: led.hex
	avrdude -p m328p -c arduino -P /dev/ttyACM0 -U flash:w:led.hex 

led.hex: led.c
	avr-gcc -g -Os -mmcu=atmega328p -c led.c
	avr-gcc -g -mmcu=atmega328p -o led.elf led.o
	avr-objcopy -j .text -j .data -O ihex led.elf led.hex
