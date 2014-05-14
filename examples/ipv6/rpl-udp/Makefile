all: udp-client udp-server
APPS=servreg-hack
CONTIKI=../../..

UIP_CONF_IPV6=1

CFLAGS+= -DUIP_CONF_IPV6_RPL

ifdef WITH_COMPOWER
APPS+=powertrace
CFLAGS+= -DCONTIKIMAC_CONF_COMPOWER=1 -DWITH_COMPOWER=1 -DQUEUEBUF_CONF_NUM=4
endif

ifdef SERVER_REPLY
CFLAGS+=-DSERVER_REPLY=$(SERVER_REPLY)
endif
ifdef PERIOD
CFLAGS+=-DPERIOD=$(PERIOD)
endif

include $(CONTIKI)/Makefile.include

OUTFILE=udp-client.avr-atmega256rfr2


rfr2flash:
        # avr-objcopy -O ihex -R .eeprom -R .fuse -R .signature $(OUTFILE) $(OUTFILE).hex
	avr-objcopy -j .text -j .data -O ihex $(OUTFILE) $(OUTFILE).hex
	avr-size -C --mcu=atmega256rfr2 $(OUTFILE)
	avrdude -c dragon_isp -P usb -p atmega256rfr2 -B3 -U flash:w:$(OUTFILE).hex
