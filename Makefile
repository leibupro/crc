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

SRCDIR := ./src
INCDIR := ./include

OBJDIR := ./obj
BINDIR := ./bin

BINARY := $(BINDIR)/crc

SRC := $(SRCDIR)/crcbit.c
SRC += $(SRCDIR)/crcbyte.c
SRC += $(SRCDIR)/util.c
SRC += $(SRCDIR)/main.c

OBJ := $(addprefix $(OBJDIR)/,$(notdir $(SRC:.c=.o)))

VPATH := $(SRCDIR)

CF += -I$(INCDIR)

.PHONY: all clean

all: $(BINARY)
	$(SIZE) $(BINARY)
	@echo -n "Used "
	@$(LD) --version | grep "gcc"

$(OBJDIR):
	mkdir $@

$(BINDIR):
	mkdir $@

$(BINARY): $(BINDIR) $(OBJ)
	$(LD) -o $@ $(LF) $(OBJ)

$(OBJDIR)/%.o: %.c $(OBJDIR)
	$(CC) $(CF) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(BINDIR)

