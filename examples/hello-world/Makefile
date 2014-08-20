OUTFILE=hello-world.avr-atmega256rfr2

CONTIKI_PROJECT = hello-world
all: $(CONTIKI_PROJECT)
#	avr-objcopy -j .text -j .data -O ihex $(OUTFILE) $(OUTFILE).hex
#	avr-objcopy -j .text -j .data -j .bootloader -O ihex $(OUTFILE) $(OUTFILE).hex
	avr-objcopy -j .text -j .data -j .bootloader -O ihex -R .fuse -R .lock -R .signature $(OUTFILE) $(OUTFILE).hex 
	avr-objcopy -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0  --no-change-warnings -O ihex $(OUTFILE) $(OUTFILE).eep || exit 0
	avr-objdump -h -S $(OUTFILE) > $(OUTFILE).lss
	avr-objcopy -O srec -R .eeprom -R .fuse -R .lock -R .signature $(OUTFILE) $(OUTFILE).srec
	avr-size $(OUTFILE)


CFLAGS+= -DUIP_CONF_IPV6_RPL

UIP_CONF_IPV6=1

CONTIKI = ../..
include $(CONTIKI)/Makefile.include

# this stuff does not seem to do much
#COFFEE_FILES = 1
#COFFEE_ADDRESS = 0x1F400


flash:
	# avr-objcopy -O ihex -R .eeprom -R .fuse -R .signature $(OUTFILE) $(OUTFILE).hex
	#avr-objcopy -j .text -j .data -O ihex $(OUTFILE) $(OUTFILE).hex
	#avr-size -C --mcu=atmega256rfr2 $(OUTFILE)
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

rfr2flash:
        # avr-objcopy -O ihex -R .eeprom -R .fuse -R .signature $(OUTFILE) $(OUTFILE).hex
	#avr-objcopy -j .text -j .data -O ihex $(OUTFILE) $(OUTFILE).hex
	#avr-size -C --mcu=atmega256rfr2 $(OUTFILE)
	# avrdude -c dragon_isp -P usb -p atmega256rfr2 -B3 -U flash:w:$(OUTFILE).hex
	# avrdude -c dragon_isp -P usb -p atmega256rfr2 -B2 -U flash:w:$(OUTFILE).hex
	avrdude -c dragon_jtag -P usb -p atmega256rfr2 -U flash:w:$(OUTFILE).hex

prog:
	gtkterm -p/dev/ttyUSB0 -s38400 -wXon -f$(OUTFILE).hex

ddd:
	/usr/local/bin/avarice --dragon --ignore-intr -B1Mhz  :4242 &
	ddd --debugger "avr-gdb -x gdb.conf"

gdb:
	/usr/local/bin/avarice --dragon --ignore-intr -B1Mhz  :4242 &
	avr-gdb -x gdb.conf

#avr-gdb hello-world.avr-atmega256rfr2

fus:
	avrdude -c dragon_jtag -P usb -p m256rfr2 -U lfuse:w:0xe2:m -U hfuse:w:0x98:m -U efuse:w:0xff:m
#	avrdude -c dragon_jtag -P usb -p m256rfr2 -U lfuse:w:0xe2:m -U hfuse:w:0x1e:m -U efuse:w:0xff:m

 
