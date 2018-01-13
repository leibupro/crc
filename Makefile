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
#  Purpose:   Some recipes to build the program(s)/
#             libraries of the CRC suite.
# 
#  
#  Remarks:   -
# 
# 
#  Date:      09/2017 
#  

SHELL := /bin/bash --login

CC := gcc
LD := $(CC)

SIZE := size

ifdef dbg
CF := -std=gnu89 -Wall -Wextra -Werror -pedantic-errors -g -O0
CF += -DDBG
CF += -DFILE_BUF_SIZE=5U
LF := -std=gnu89 -lrt -Wl,-v
else
CF := -std=gnu89 -Wall -Wextra -Werror -pedantic-errors -O3 -funroll-loops -flto
LF := -std=gnu89 -flto -O3 -lrt -Wl,-v
endif

LIBCF := -fPIC

SRCDIR  := ./src
INCDIRS := -I./include

OBJDIR    := ./obj
LIBOBJDIR := ./libobj
BINDIR    := ./bin
LIBDIR    := ./lib

BINARY := $(BINDIR)/crc

LIBFILE    := libcrc.so.1.0.1
LIBDYN     := $(LIBDIR)/$(LIBFILE)
LIBDYNNAME := libcrc.so.1
LIBLINK_1  := $(LIBDIR)/libcrc.so
LIBLINK_2  := $(LIBDIR)/$(LIBDYNNAME)

SRC := $(SRCDIR)/crcbit.c  \
       $(SRCDIR)/crcbyte.c \
       $(SRCDIR)/util.c    \
       $(SRCDIR)/main.c

LIBSRC := $(SRCDIR)/crcbyte.c \
          $(SRCDIR)/util.c

OBJ    := $(addprefix $(OBJDIR)/,$(notdir $(SRC:.c=.o)))
LIBOBJ := $(addprefix $(LIBOBJDIR)/,$(notdir $(LIBSRC:.c=.o)))

VPATH := $(SRCDIR)


.PHONY: all library clean

all: $(BINARY)
	$(SIZE) $(BINARY)
	@echo -n "Used "
	@$(LD) --version | grep "gcc"

library: $(LIBDYN)
	$(SIZE) $<
	@echo -n "Used "
	@$(LD) --version | grep "gcc"

$(OBJDIR):
	mkdir $@

$(LIBOBJDIR):
	mkdir $@

$(BINDIR):
	mkdir $@

$(LIBDIR):
	mkdir $@

$(BINARY): $(BINDIR) $(OBJ)
	$(LD) -o $@ $(LF) $(OBJ)

$(LIBDYN): $(LIBDIR) $(LIBOBJ)
	$(LD) -shared -Wl,-soname,$(LIBDYNNAME) -o $@ $(LIBOBJ)
	ln -s $(LIBFILE) $(LIBLINK_1)
	ln -s $(LIBFILE) $(LIBLINK_2)

$(OBJDIR)/%.o: %.c $(OBJDIR)
	$(CC) $(CF) $(INCDIRS) -c $< -o $@

$(LIBOBJDIR)/%.o: %.c $(LIBOBJDIR)
	$(CC) $(CF) $(LIBCF) $(INCDIRS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(BINDIR) $(LIBOBJDIR) $(LIBDIR)

