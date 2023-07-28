ROOTDIR := $(realpath .)
INCDIR = $(ROOTDIR)/include
BLDIR = $(ROOTDIR)/build
LOGDIR = $(ROOTDIR)/logs
TESTDIR = $(ROOTDIR)/test

BLDRULES = $(ROOTDIR)/rules.mk

# toolchain
CC = @gcc -std=c99
LD = @gcc

ifeq ($(OS),Windows_NT)

AR = @x86_64-w64-mingw32-ar -rcs --thin

MKDIR = @mkdir_

else

AR = @ar -rcsT

MKDIR = @mkdir

endif


# compiler flags
CFLAGS = -c -pedantic -Wall -Wextra -Werror -I $(INCDIR)
DBG_CFLAGS = -g -DDEBUG
RLS_CFLAGS = -s -fdata-sections -ffunction-sections -O3

LDLIBS =
LDFLAGS = -static

C_OUT = -o

RED := "\033[0;91m"	
YELLOW := "\033[0;33m"
RESET := "\033[0m"

define print_info
	@echo -e [i] $$1 $1
endef

define print_warning
	@echo -e $(YELLOW)[!] $$1 $1 $(RESET)	
endef

define print_error
	@echo -e $(RED)[-] $$1 $1 $(RESET)	
endef
