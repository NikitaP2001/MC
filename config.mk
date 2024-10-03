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
ECHO = echo -e

else

AR = @ar -rcsT

MKDIR = @mkdir
ECHO = echo

endif


# compiler flags
CFLAGS = -c -pedantic -I $(INCDIR)
DBG_CFLAGS = -g -DDEBUG -Wall -Wextra -Werror
RLS_CFLAGS = -s -fdata-sections -ffunction-sections -O3 -DNDEBUG

LDLIBS =
LDFLAGS = -static

C_OUT = -o

RED := "\033[0;91m"	
YELLOW := "\033[0;33m"
RESET := "\033[0m"

define print_info
	@$(ECHO) [i] $$1 $1
endef

define print_warning
	@$(ECHO) $(YELLOW)[!] $$1 $1 $(RESET)	
endef

define print_error
	@$(ECHO) $(RED)[-] $$1 $1 $(RESET)	
endef
