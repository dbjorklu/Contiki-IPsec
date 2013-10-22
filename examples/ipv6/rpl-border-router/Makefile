ALLOW_OLD_PLATFORMS=1

MCU=atmega256rfr2


CONTIKI_PROJECT=border-router
all: $(CONTIKI_PROJECT) 

CONTIKI=../../..

WITH_UIP6=1
UIP_CONF_IPV6=1
CFLAGS+= -DUIP_CONF_IPV6_RPL

#linker optimizations
SMALL=1

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
PROJECT_SOURCEFILES += slip-bridge.c

#Simple built-in webserver is the default.
#Override with make WITH_WEBSERVER=0 for no webserver.
#WITH_WEBSERVER=webserver-name will use /apps/webserver-name if it can be
#found in the /apps, /platform/$(TARGET)/apps/, or current directory (in that order).
# WITH_WEBSERVER=webserver for /apps/webserver
# WITH_WEBSERVER=raven-webserver for /platform/avr-raven/apps/raven-webserver/
#make clean before changing webservers!

#Note /apps/webserver contains a 2500 byte style sheet which is a severe test
#of the slip connection. Large MSS together with low baud rates without flow
#control will overrun the transmit buffer when the style sheet is requested.

WITH_WEBSERVER=1
ifeq ($(WITH_WEBSERVER),1)
CFLAGS += -DWEBSERVER=1
PROJECT_SOURCEFILES += httpd-simple.c
else ifneq ($(WITH_WEBSERVER), 0)
APPS += $(WITH_WEBSERVER)
CFLAGS += -DWEBSERVER=2
endif

ifeq ($(PREFIX),)
 PREFIX = aaaa::1/64
endif

include $(CONTIKI)/Makefile.include

OUTFILE=border-router.avr-atmega256rfr2


$(CONTIKI)/tools/tunslip6:	$(CONTIKI)/tools/tunslip6.c
	(cd $(CONTIKI)/tools && $(MAKE) tunslip6)

load:
	mc1322x-load.pl -f border-router_econotag.bin -t /dev/ttyUSB0

rfr2flash:
        # avr-objcopy -O ihex -R .eeprom -R .fuse -R .signature $(OUTFILE) $(OUTFILE).hex
	avr-objcopy -j .text -j .data -O ihex $(OUTFILE) $(OUTFILE).hex
	avr-size -C --mcu=m256rfr2 $(OUTFILE)
	avrdude -c dragon_isp -P usb -p m256rfr2 -B3 -U flash:w:$(OUTFILE).hex

# read fuse
# extendend
read_efuse:
	avrdude -c dragon_isp -P usb -p m256rfr2 -U efuse:r:efuse.hex:i
# high
read_hfuse:
	avrdude -c dragon_isp -P usb -p m256rfr2 -U hfuse:r:hfuse.hex:i
# low
read_lfuse:
	avrdude -c dragon_isp -P usb -p m256rfr2 -U lfuse:r:lfuse.hex:i

# write fuse
# extendend
write_efuse:
	avrdude -c dragon_isp -P usb -p m256rfr2 -U efuse:w:fuse/efuse.hex:i
# high
write_hfuse:
	avrdude -c dragon_isp -P usb -p m256rfr2 -U hfuse:w:fuse/hfuse.hex:i
# low
write_lfuse:
	avrdude -c dragon_isp -P usb -p m256rfr2 -U lfuse:w:fuse/lfuse.hex:i


connect-router:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 -B57600 $(PREFIX)
#	sudo $(CONTIKI)/tools/tunslip6 -B38400 $(PREFIX)

connect-router-cooja:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 -a 127.0.0.1 $(PREFIX)
