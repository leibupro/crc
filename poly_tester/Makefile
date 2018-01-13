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
CF := -std=gnu89 -Wall -Wextra -Werror -pedantic-errors -g -O0
CF += -DDBG
LF := -std=gnu89 -lrt -Wl,-v
else
CF := -std=gnu89 -Wall -Wextra -Werror -pedantic-errors -O3 -funroll-loops -flto
LF := -std=gnu89 -flto -O3 -lrt -Wl,-v
endif

INCDIRS := -I../include
LIBDIRS := -L../lib
RPATH   := -Wl,-rpath=$(abspath ../lib)

SRCDIR := ./src

LIBNAME := crc

LIBFILE := ../lib/libcrc.so.1.0.1

OBJDIR := ./obj
BINDIR := ./bin

BINARY := $(BINDIR)/polytstr

SRC := $(SRCDIR)/polytstr.c

OBJ := $(addprefix $(OBJDIR)/,$(notdir $(SRC:.c=.o)))

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
	cd .. && $(MAKE) library


$(BINARY): $(LIBFILE) $(BINDIR) $(OBJ)
	$(LD) -o $@ $(LF) $(RPATH) $(OBJ) $(LIBDIRS) -l$(LIBNAME) -lpthread


$(OBJDIR)/%.o: %.c $(OBJDIR)
	$(CC) $(CF) $(INCDIRS) -c $< -o $@


clean:
	rm -rf $(OBJDIR) $(BINDIR)

