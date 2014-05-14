CONTIKI = ../../..
APPS = powertrace collect-view
CONTIKI_PROJECT = udp-sender udp-sink
PROJECT_SOURCEFILES += collect-common.c

UIP_CONF_IPV6=1
CFLAGS+= -DUIP_CONF_IPV6_RPL

ifdef PERIOD
CFLAGS=-DPERIOD=$(PERIOD)
endif

all: $(CONTIKI_PROJECT)

include $(CONTIKI)/Makefile.include


flash_sink:
        # avr-objcopy -O ihex -R .eeprom -R .fuse -R .signature $(OUTFILE) $(OUTFILE).hex
	avr-objcopy -j .text -j .data -O ihex udp-sink.avr-atmega256rfr2 udp-sink.avr-atmega256rfr2.hex
	avr-size -C --mcu=atmega256rfr2 udp-sink.avr-atmega256rfr2
	avrdude -c dragon_isp -P usb -p atmega256rfr2 -B3 -U flash:w:udp-sink.avr-atmega256rfr2.hex

flash_sender:
        # avr-objcopy -O ihex -R .eeprom -R .fuse -R .signature $(OUTFILE) $(OUTFILE).hex
	avr-objcopy -j .text -j .data -O ihex udp-sender.avr-atmega256rfr2 udp-sender.avr-atmega256rfr2.hex
	avr-size -C --mcu=atmega256rfr2 udp-sender.avr-atmega256rfr2
	avrdude -c dragon_isp -P usb -p atmega256rfr2 -B3 -U flash:w:udp-sender.avr-atmega256rfr2.hex


