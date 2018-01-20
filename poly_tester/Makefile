# 
#  ----------------------------------------------------------------------------
#  "THE BEER-WARE LICENSE" (Revision 42):
#  <pl@vqe.ch> wrote this file.  As long as you retain this notice you
#  can do whatever you want with this stuff. If we meet some day, and you think
#  this stuff is worth it, you can buy me a beer in return.   P. Leibundgut
#  ----------------------------------------------------------------------------
# 
#  File:      Makefile
#  
# 
#  Purpose:   Some recipes to build the polynomial tester
#             for the crc library.
# 
#  
#  Remarks:   - This test program requires the crc library.
#
#             - link against -lpthread and -lcrc
# 
# 
#  Date:      01/2018 
#

SHELL := /bin/bash --login

CC := gcc
LD := $(CC)

SIZE := size

MAKE := colormake

ifdef dbg
include ../flags_debug.mk
SUBMAKE_VAR := dbg=1
else
include ../flags_optimize.mk
SUBMAKE_VAR := 
endif

# needed for Thaler's mtimer.h
CF += -D__linux

INCDIRS := -I../include \
           -I./include
LIBDIRS := -L../lib
RPATH   := -Wl,-rpath="$(abspath ../lib)"

SRCDIR := ./src

LIBNAME := crc

LIBFILE := ../lib/libcrc.so.1.0.1

OBJDIR := ./obj
BINDIR := ./bin

BINARY := $(BINDIR)/polytstr

SRC := $(SRCDIR)/polytstr.c \
       $(SRCDIR)/hamming.c

OBJ := $(addprefix $(OBJDIR)/,$(notdir $(SRC:.c=.o)))

# for cleaning the library to rebuild
LIB_BUNDLE := ../lib \
              ../libobj

VPATH := $(SRCDIR)


.PHONY: all clean

all: $(BINARY)
	$(SIZE) $(BINARY)
	@echo -n "Used "
	@$(LD) --version | grep "gcc"


$(OBJDIR):
	mkdir $@

$(BINDIR):
	mkdir $@


$(LIBFILE):
	cd .. && $(MAKE) library $(SUBMAKE_VAR)


$(BINARY): $(LIBFILE) $(BINDIR) $(OBJ)
	$(LD) -o $@ $(LF) $(RPATH) $(OBJ) $(LIBDIRS) -l$(LIBNAME) -lpthread


$(OBJDIR)/%.o: %.c $(OBJDIR)
	$(CC) $(CF) $(INCDIRS) -c $< -o $@


clean:
	rm -rf $(OBJDIR) $(BINDIR) $(LIB_BUNDLE)

