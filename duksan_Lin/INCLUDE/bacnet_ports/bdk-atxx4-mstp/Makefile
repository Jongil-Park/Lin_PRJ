###############################################################################
# Makefile for LM Room Controller - AVR
###############################################################################

## General Flags
MCU = atmega644p
AVRDUDE_MCU = m644p
# ATmega644 bootload is at word address 7000h, 7800h, 7C00h, or 7E00h
# Double that value to get the byte address
BOOTLOAD = 0xF800
TARGET = bacnet
## Tools
CC = avr-gcc
AR = avr-ar
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
AVRDUDE = avrdude
LINT = splint

# programmer id--check the avrdude for complete list
# of available opts.  These should include stk500,
# avr910, avrisp, bsd, pony and more.  Set this to
# one of the valid "-c PROGRAMMER-ID" values 
# described in the avrdude info page.
# jtag2fast = Atmel JTAG ICE mkII, running at 115200 Bd
# jtag2slow = Atmel JTAG ICE mkII, running at 19200 Bd
# avrispmkII = AVR ISP MKII
# avr109 = bootloader
#AVRDUDE_PROGRAMMERID = avr109
#AVRDUDE_PROGRAMMERID = jtag2fast
AVRDUDE_PROGRAMMERID = avrispmkII
#AVRDUDE_PROGRAMMERID = dragon_isp
#AVRDUDE_PROGRAMMERID = dragon_jtag
#
# port--serial or parallel port to which your 
# hardware programmer is attached
# usb can just be usb
AVRDUDE_PORT = usb
#AVRDUDE_PORT = /dev/ttyUSB0

# Source locations
BACNET_CORE = ../../src
BACNET_INCLUDE = ../../include
BACNET_DEMO = ../../demo

# local files for this project
CSRC = main.c \
	fuses.c \
	init.c \
	stack.c \
	adc.c \
	input.c \
	serial.c \
	rs485.c \
	timer2.c \
	led.c \
	eeprom.c \
	seeprom.c \
	dlmstp.c \
	h_wp.c \
	h_rp.c \
	h_rpm.c \
	h_rd.c \
	device.c \
	ai.c \
	av.c \
	bi.c \
	bo.c

# common demo files needed
DEMOSRC = $(BACNET_DEMO)/handler/txbuf.c \
	$(BACNET_DEMO)/handler/h_npdu.c \
	$(BACNET_DEMO)/handler/h_whois.c \
	$(BACNET_DEMO)/handler/h_dcc.c \
	$(BACNET_DEMO)/handler/s_iam.c \
	$(BACNET_DEMO)/handler/noserv.c

# core BACnet stack files
CORESRC =  \
	$(BACNET_CORE)/fifo.c \
	$(BACNET_CORE)/memcopy.c \
	$(BACNET_CORE)/crc.c \
	$(BACNET_CORE)/apdu.c \
	$(BACNET_CORE)/npdu.c \
	$(BACNET_CORE)/bacdcode.c \
	$(BACNET_CORE)/bacint.c \
	$(BACNET_CORE)/bacreal.c \
	$(BACNET_CORE)/bacstr.c \
	$(BACNET_CORE)/iam.c \
	$(BACNET_CORE)/dcc.c \
	$(BACNET_CORE)/rp.c \
	$(BACNET_CORE)/rd.c \
	$(BACNET_CORE)/rpm.c \
	$(BACNET_CORE)/wp.c \
	$(BACNET_CORE)/whois.c \
	$(BACNET_CORE)/bacaddr.c \
	$(BACNET_CORE)/abort.c \
	$(BACNET_CORE)/reject.c \
	$(BACNET_CORE)/bacerror.c \
	$(BACNET_CORE)/bacapp.c

#	$(BACNET_CORE)/version.c
#	$(BACNET_CORE)/bacprop.c \
#	$(BACNET_CORE)/bactext.c \
#	$(BACNET_CORE)/datetime.c \
#	$(BACNET_CORE)/indtext.c \
#	$(BACNET_CORE)/bigend.c \
#	$(BACNET_CORE)/arf.c \
#	$(BACNET_CORE)/awf.c \
#	$(BACNET_CORE)/cov.c \
#	$(BACNET_CORE)/iam/iam_client.c \
#	$(BACNET_CORE)/ihave.c \
#	$(BACNET_CORE)/timesync.c \
#	$(BACNET_CORE)/whohas.c \
#	$(BACNET_CORE)/filename.c \
#	$(BACNET_CORE)/tsm.c \
#	$(BACNET_CORE)/address.c \

## Include Directories
INCLUDES = -I. -I$(BACNET_INCLUDE)

# Source to Object conversion
COBJ = $(CSRC:%.c=%.o)
DEMOOBJ = $(DEMOSRC:.c=.o)
COREOBJ = $(CORESRC:.c=.o)

LIBRARY = lib$(TARGET).a

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

# define something from the Makefile or batch file
DEFINES = 

OPTIMIZE_FLAGS = -mcall-prologues
OPTIMIZE_FLAGS += -finline-functions-called-once
# default is for debugging from AVR Studio
OPTIMIZATION = -O0
DEBUGGING = -g
ifeq (${BUILD},debug)
OPTIMIZATION = -O0
DEBUGGING = -g
endif
ifeq (${BUILD},release)
OPTIMIZATION = -Os $(OPTIMIZE_FLAGS)
DEBUGGING =
endif

## BACnet options
BFLAGS = -DBACDL_MSTP
BFLAGS += -DMAX_APDU=128
BFLAGS += -DBIG_ENDIAN=0
BFLAGS += -DMAX_TSM_TRANSACTIONS=0
#BFLAGS += -DCRC_USE_TABLE
BFLAGS += -DBACAPP_BOOLEAN
BFLAGS += -DBACAPP_REAL
BFLAGS += -DBACAPP_OBJECT_ID
BFLAGS += -DBACAPP_UNSIGNED
BFLAGS += -DBACAPP_ENUMERATED
BFLAGS += -DBACAPP_CHARACTER_STRING
BFLAGS += -DWRITE_PROPERTY

## Compile options for C files
CFLAGS = $(COMMON)
CFLAGS += $(DEFINES)
CFLAGS += $(DEBUGGING)
# dead code removal
CFLAGS += -ffunction-sections -fdata-sections
# General flags
CFLAGS += -funsigned-char
CFLAGS += -funsigned-bitfields
CFLAGS += -fpack-struct
CFLAGS += -fshort-enums
# warnings
CFLAGS += -Wall
CFLAGS += -Wstrict-prototypes
# put it all together
CFLAGS += -gdwarf-2 $(BFLAGS) $(OPTIMIZATION)
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
#dead code removal
#LDFLAGS += -Wl,-nostartfiles,-nostdlib
LDFLAGS += -Wl,--gc-sections,-static
LDFLAGS += -Wl,--section-start=.bootloader=$(BOOTLOAD) 
LDFLAGS += -Wl,-Map=$(TARGET).map
LDFLAGS += -Wl,-L=.,-l$(TARGET)

## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature
HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings

HEX_FUSE_FLAGS = -j .fuse
HEX_FUSE_FLAGS += --change-section-lma .fuse=0 --no-change-warnings

AVRDUDE_FLAGS = -c $(AVRDUDE_PROGRAMMERID)
AVRDUDE_FLAGS += -p $(AVRDUDE_MCU) 
AVRDUDE_FLAGS += -P $(AVRDUDE_PORT)

# Fuse high byte (0=enable,1=disable):
# 0x93 = 1 0 0 1   0 0 1 1
# 0x17 = 0 0 0 1   0 1 1 1 - default
#        ^ ^ ^ ^   ^ \+/ ^ 
#        | | | |   |  |  |---- BOOTRST (Enable Bootloader Reset Vector)
#        | | | |   |  +------- BOOTSZ 1..0 (Select Boot Size)
#        | | | |   |  +------- [00=4k, 01=2k, 10=1k, 11=512]
#        | | | |   +---------- EESAVE (Enable preserve EEPROM on Chip Erase)
#        | | | +-------------- WDTON (watchdog timer always on)
#        | | +---------------- SPIEN (Enable Serial Program / Data Downloading)
#        | +------------------ JTAGEN (Enable JTAG)
#        +-------------------- OCDEN (Enable OCD)
#
# Fuse low byte (0=enable,1=disable):
# 0xD7 = 1 1 0 1   0 1 1 1
# 0xC7 = 1 1 0 0   0 1 1 1
# 0x62 = 0 1 1 0   0 0 1 0 - default
#        ^ ^ \+/   \--+--/
#        | |  |       +------- CKSEL 3..0 (Select Clock Source)
#        | |  |       +------- [1111-1000=Low Power Crystal Oscillator]
#        | |  |       +------- [0111-0110=Full Swing Crystal Oscillator]
#        | |  |       +------- [0101-0100=Low Frequency Crystal Oscillator]
#        | |  |       +------- [0011=Internal 128kHz RC Oscillator]
#        | |  |       +------- [0010=Calibrated Internal RC Oscillator]
#        | |  |       +------- [0000=External Clock]
#        | |  +--------------- SUT 1..0 (Start up Time selection)
#        | |  +--------------- [CKSEL0=0:14CK+ 00=4.1ms,01=65ms,10=0ms,11=4.1ms]
#        | |  +--------------- [CKSEL0=1:14CK+ 00=65ms,01=BOD,10=4.1ms,11=65ms]
#        | +------------------ CKOUT (clock output on CKOUT pin)
#        +-------------------- CKDIV8 (divide clock by 8)
#
# Fuse extended byte (0=enable,1=disable):
# 0xFC = 1 1 1 1   1 1 0 0
# 0xFC = 1 1 1 1   1 1 1 1 - default
#        ^ ^ ^ ^   ^ \-+-/ 
#        | | | |   |   +------ BODLEVEL 2..0 (brownout trigger level)
#        | | | |   |   +------ [100=4.3V, 101=2.7V, 110=1.8V, 111=disabled]
#        | | | |   +---------- 
#        | | | +-------------- 
#        | | +---------------- 
#        | +------------------ 
#        +--------------------
AVRDUDE_DEFAULT_FUSES = -U hfuse:w:0x17:m -U lfuse:w:0x62:m -U efuse:w:0xFF:m

AVRDUDE_WRITE_FUSES = -U hfuse:w:0x93:m -U lfuse:w:0xD7:m -U efuse:w:0xFC:m

AVRDUDE_BOOTL_FUSES = -U hfuse:w:0x92:m -U lfuse:w:0xD7:m -U efuse:w:0xFC:m

AVRDUDE_READ_FUSES = -U lfuse:r:-:h -U hfuse:r:-:h -U efuse:r:-:h

AVRDUDE_WRITE_FLASH = -e -U flash:w:$(TARGET).hex

AVRDUDE_INSTALL = $(AVRDUDE_WRITE_FLASH)
#AVRDUDE_INSTALL += $(AVRDUDE_WRITE_FUSES)

## Objects that must be built in order to link
OBJECTS = $(COBJ) $(DEMOOBJ)

## Build
TARGET_ELF=$(TARGET).elf

all: $(LIBRARY) \
	$(TARGET_ELF) \
	$(TARGET).hex \
	$(TARGET).eep \
	$(TARGET).lst \
	size Makefile

##Link
$(TARGET_ELF): $(OBJECTS) $(LIBRARY)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

%.hex: $(TARGET_ELF) 
	$(OBJCOPY) -O ihex $(HEX_FLASH_FLAGS) $< $@

%.eep: $(TARGET_ELF)
	-$(OBJCOPY) $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lst: $(TARGET_ELF)
	$(OBJDUMP) -h -S $< > $@

lib: $(LIBRARY)

$(LIBRARY): $(COREOBJ) Makefile
	$(AR) rcs $@ $(COREOBJ)
	$(OBJDUMP) --syms $@ > $(LIBRARY:.a=.lst)

%.o: %.c
	$(CC) -c $(INCLUDES) $(CFLAGS) $*.c -o $@

size: ${TARGET_ELF}
	@echo
	@${SIZE} ${TARGET_ELF}

lint:
	$(LINT) $(BFLAGS) $(CSRC) 

install: $(TARGET_ELF) 
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_INSTALL)
	
writefuses:
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FUSES)	

bootloadfuses:
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_BOOTL_FUSES)	

defaultfuses:
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_DEFAULT_FUSES)	

showfuses:
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_READ_FUSES)	

bootloader:
	make -C bootloader all

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) $(TARGET_ELF) dep/*
	-rm -rf $(LIBRARY) $(COREOBJ) $(LIBRARY:.a=.lst)
	-rm -rf $(TARGET).hex $(TARGET).eep $(TARGET).lst $(TARGET).map

## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)
