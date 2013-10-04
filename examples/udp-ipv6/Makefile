OUTFILE=udp-server.avr-raven

MCU=atmega1284p


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

