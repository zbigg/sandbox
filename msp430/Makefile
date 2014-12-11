#
# MSP430 makefile for simple programs without Energia API at all
#

# some notes about layout of developer tools
# compilation flags "reverse-engineered" from energia:
# 
#   c:\progs\energia-0101E0013\hardware\tools\msp430\bin
#     msp430-c++.exe 
#      ....
# http://43oh.com/2010/11/a-step-by-step-guide-msp430-programming-under-linux/
# about flasher:
# - http://processors.wiki.ti.com/index.php/MSP430_Flasher_-_Command_Line_Programmer
# - http://www.embeddedrelated.com/showarticle/420.php

TOOLS_HOME = c:/progs/energia-0101E0013/hardware/tools/msp430

CROSS_COMPILE=msp430-

FLASHER = /c/progs/MSP430Flasher_1.3.3/MSP430Flasher.exe
CC=$(TOOLS_HOME)/bin/$(CROSS_COMPILE)gcc
CXX=$(TOOLS_HOME)/bin/$(CROSS_COMPILE)g++
OBJCOPY=$(TOOLS_HOME)/bin/$(CROSS_COMPILE)objcopy

CFLAGS = -Os -w -ffunction-sections -fdata-sections -mmcu=msp430g2553 -DF_CPU=16000000L
CXXFLAGS = $(CFLAGS)
LDFLAGS  = -mmcu=msp430g2553 -Os -Wl,-gc-sections,-u,main

all:  programs

#%.o: %.cpp
#	$(CC) $(CXXFLAGS) -c $< -o $@

%.elf: %.o
	$(CC) $(LDFLAGS)  $< -o $@

%.hex: %.elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

programs = msp430_hello_led.hex msp430_hello_world_led_interrupts.hex
	
programs: $(programs)

flash:
	$(FLASHER) -w $(prog) -e ERASE_ALL -z "[VCC]"
flash-%: %.hex
	$(FLASHER) -w $< -e ERASE_ALL -z "[VCC]"

clean:
	rm -rf *.o *.hex *.elf