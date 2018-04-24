################################################################################
# Makefile for XMC4500 RelaxKit using uCOS-III
# v1, 02/2018
# Martin Horauer, UAS Technikum Wien
#
# Supported: Windows, Linux, OSX
# Requirements:
# * GCC ARM https://launchpad.net/gcc-arm-embedded/+download
# * SEGGER JLINK https://www.segger.com/jlink-software.html
# * DOXYGEN http://www.stack.nl/~dimitri/doxygen/

################################################################################
# USAGE
# -----
# make           .... build the program image
# make debug     .... build the program image and invoke gdb
# make flash     .... build an flash the application
# make erase     .... erase the target device
# make doc       .... run doxygen - output will be in > doc
# make clean     .... remove intermediate and generated files

################################################################################
# define the name of the generated output file
#
TARGET        = main

################################################################################
# below only edit with care
#
VENDOR        = Infineon

################################################################################
# define the following symbol -D JLINK_RTT to enable JLINK_RTT tracing
#                             -D SEMI_HOSTING to enable semi hosted tracing
#                             comment the line to disable tracing
TRACE         = -D SEMI_HOSTING

################################################################################
# TOOLS & ARGS
#
SRC           = $(wildcard *.cpp)
TERMINAL      = gnome-terminal
TOOLCHAIN     = GCC_ARM
BOARD         = XMC_4500_RELAX_KIT
BUILDDIR      = BUILD
DOCDIR        = DOC
MBED          = mbed
GDB           = arm-none-eabi-gdb

# DETERMINE OS
ifdef SystemRoot
  RM = del /Q
  FixPath = $(subst /,\,$1)
else
  RM = rm -rf
  FixPath = $1
endif

GDB_ARGS      = -ex "target remote :2331"
GDB_ARGS     += -ex "monitor reset"
GDB_ARGS     += -ex "load"
GDB_ARGS     += -ex "monitor reset"

################################################################################
# SEMI_HOSTED DEBUGGING
GDB_ARGS     += -ex "monitor SWO EnableTarget 16000000 0 1 0"

# RTT OPTION
#GDB_ARGS     += -ex "monitor exec SetRTTAddr 0x20000000"
#GDB_ARGS     += -ex "monitor exec SetRTTSearchRanges 0x20000000 0x1000"

################################################################################
# BUILD RULES
all: $(SRC) build

################################################################################
# CREATE A DEBUG VESRION
build: $(SRC)
	@echo "----------------------------------------------------------------------"
	@echo "Building with DEBUG Symbols"
	@echo ""
	$(MBED) compile -m $(BOARD) -t $(TOOLCHAIN) -N main --profile mbed-os/tools/profiles/debug.json
	@echo ""

################################################################################
# CREATE A RELEASE VESRION
release: $(SRC)
	@echo "----------------------------------------------------------------------"
	@echo "Build a RELEASE version"
	@echo ""
	$(MBED) compile -m $(BOARD) -t $(TOOLCHAIN) -N main
	@echo ""

################################################################################
# DEBUG RULES
debug: build $(BUILDDIR)/$(BOARD)/$(TOOLCHAIN)/$(TARGET).elf
ifdef SystemRoot
	@call start JLinkGDBServer -Device XMC4500-1024 -if SWD
else
	$(TERMINAL) -e "JLinkGDBServer -Device XMC4500-1024 -if SWD" &
	sleep 1 && $(TERMINAL) -e "telnet 127.0.0.1 2333" & 
endif
	$(GDB) -q  $(BUILDDIR)/$(BOARD)/$(TOOLCHAIN)/$(TARGET).elf $(GDB_ARGS)

################################################################################
# FLASH RULES
flash: build $(BUILDDIR)/$(BOARD)/$(TOOLCHAIN)/$(TARGET).hex
	echo -e 'speed 4000\nconnect\nh\nloadbin $(BUILDDIR)/$(BOARD)/$(TOOLCHAIN)/$(TARGET).hex,0xC000000\nr\ng\nq' | JLinkExe -Device XMC4500-1024 -if SWD

################################################################################
# ERASE DEVICE
erase:
	echo -e 'speed 4000\nconnect\nerase\nr\nq' | JLinkExe -Device XMC4500-1024 -if SWD

################################################################################
# DOCUMENTATION RULES
doc: $(SRC)
	doxygen

################################################################################
# CLEAN RULES
clean:
	$(RM) $(call FixPath, *.pyc)
	$(RM) $(call FixPath, ${BUILDDIR}/*)
	$(RM) $(call FixPath, ${DOCDIR}/html/*)

################################################################################
# EOF
################################################################################
