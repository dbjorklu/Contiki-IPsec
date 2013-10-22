#OUTFILE=udp-server.avr-raven
OUTFILE=udp-client.avr-atmega256rfr2

# MCU=atmega1284p
MCU=atmega256rfr2


all: udp-server udp-client

ravenhex:
	avr-objcopy -O ihex -R .signature -R .fuse -R .eeprom $(OUTFILE) $(OUTFILE).hex
	avr-size -C --mcu=$(MCU) $(OUTFILE)


UIP_CONF_IPV6=1

CONTIKI = ../..
include $(CONTIKI)/Makefile.include

load:
	mc1322x-load.pl -f  udp-client_econotag.bin -t /dev/ttyUSB0


ravenflash:
	avrdude -c dragon_jtag -P usb -p $(MCU) -U flash:w:$(OUTFILE).hex

rfr2flash:
        # avr-objcopy -O ihex -R .eeprom -R .fuse -R .signature $(OUTFILE) $(OUTFILE).hex
	avr-objcopy -j .text -j .data -O ihex $(OUTFILE) $(OUTFILE).hex
	avr-size -C --mcu=m256rfr2 $(OUTFILE)
	avrdude -c dragon_isp -P usb -p m256rfr2 -B3 -U flash:w:$(OUTFILE).hex
