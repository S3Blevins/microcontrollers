#
# Makefile for the C demo
#
# Targets:
#   all         generates flash file
#   install     downloads elf file to mcu
#

PROJNAME    = lab_5
FILENAME    = main
OBJFILES    = main.o neo_color.o
DBGPATH     = Debug
DBGOBJS     = $(addprefix $(DBGPATH)/, $(OBJFILES))

MCU         = atmega1284

CCLD        = avr-gcc
#CCFLAGS    = -mmcu=$(MCU) -Wall -Os -gstabs
CCFLAGS     = -mmcu=$(MCU) -Wall -Os -frename-registers
CCFLAGS     += -fshort-enums -fpack-struct
#CCFLAGS    += -dwarf -g2
LDFLAGS     = -mmcu=$(MCU) 

PORT        = usb
PROGRAMMER  = dragon_jtag
PROG        = avrdude
PRFLAGS     = -v -c $(PROGRAMMER) -p $(PARTNO) -P$(PORT)

OBJCOPY     = avr-objcopy
OBJFLAGS 	= -j .text -j .data -O ihex

all: $(DBGOBJS) $(DBGPATH)/$(PROJNAME).hex $(PROJNAME).elf $(DBGPATH)/$(PROJNAME).lss

$(PROJNAME).elf: $(DBGOBJS)
	$(CCLD) $(LDFLAGS) -o $@ $^

$(DBGPATH)/%.o: %.s
	$(CCLD) $(CCFLAGS) -c -o $@ $<
$(DBGPATH)/%.o: %.c
	$(CCLD) $(CCFLAGS) -c -o $@ $<

$(DBGPATH)/$(PROJNAME).hex: $(PROJNAME).elf
	$(OBJCOPY) $(OBJFLAGS) $<  $@

$(DBGPATH)/$(PROJNAME).lss: $(PROJNAME).elf
	avr-objdump -h -S $< > $@

clean:
	rm -f $(DBGPATH)/$(PROJNAME).hex $(DBGPATH)/$(PROJNAME).elf $(DBGOBJS) $(DBGPATH)/$(PROJNAME).lss
