CONTIKI_PROJECT = hello-world
all: $(CONTIKI_PROJECT)

#UIP_CONF_IPV6=1

CONTIKI = ../..
include $(CONTIKI)/Makefile.include

OUTFILE=hello-world.avr-atmega256rfr2

flash:
	# avr-objcopy -O ihex -R .eeprom -R .fuse -R .signature $(OUTFILE) $(OUTFILE).hex
	avr-objcopy -j .text -j .data -O ihex $(OUTFILE) $(OUTFILE).hex
	avr-size -C --mcu=atmega256rfr2 $(OUTFILE)
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
	avr-objcopy -j .text -j .data -O ihex $(OUTFILE) $(OUTFILE).hex
	avr-size -C --mcu=atmega256rfr2 $(OUTFILE)
	# avrdude -c dragon_isp -P usb -p atmega256rfr2 -B3 -U flash:w:$(OUTFILE).hex
	avrdude -c dragon_isp -P usb -p atmega256rfr2 -B2 -U flash:w:$(OUTFILE).hex
