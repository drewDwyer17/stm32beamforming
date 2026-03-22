BUILD_DIR = build
PROJECT = stm32beamforming

SHARED_DIR = ../common
CFILES += src/main.c 

CFILES += src/signalMgr.c
CFILES += src/phaseShifter.c
HFILES += include/SignalMgr.h
HFILES += include/PhaseShifter.h


DEVICE = stm32f030c8

# You shouldn't have to edit anything below here.
VPATH += $(SHARED_DIR)
INCLUDES += $(patsubst %,-I%, . $(SHARED_DIR))
INCLUDES += -Iinclude
OPENCM3_DIR=../libopencm3
OPENCM3_DEFS = -DSTM32F0

include $(OPENCM3_DIR)/mk/genlink-config.mk
include ../rules.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

all: $(BUILD_DIR) $(PROJECT).elf
	mv $(PROJECT).elf $(BUILD_DIR)/
	mv $(PROJECT).bin $(BUILD_DIR)/


##from the makerules. mkrules expects these to be in the current make file: 
# This version of rules.mk expects the following to be defined before
# inclusion..
### REQUIRED ###
# OPENCM3_DIR - duh
# PROJECT - will be the basename of the output elf, eg usb-gadget0-stm32f4disco
# CFILES - basenames only, eg main.c blah.c
# CXXFILES - same for C++ files. Must have cxx suffix!
# DEVICE - the full device name, eg stm32f405ret6
#  _or_
# LDSCRIPT - full path, eg ../../examples/stm32/f4/stm32f4-discovery/stm32f4-discovery.ld
# OPENCM3_LIB - the basename, eg: opencm3_stm32f4
# OPENCM3_DEFS - the target define eg: -DSTM32F4
# ARCH_FLAGS - eg, -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16
#    (ie, the full set of cpu arch flags, _none_ are defined in this file)