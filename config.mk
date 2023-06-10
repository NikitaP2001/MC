# toolchain
CC= @gcc -std=c99
AR = @x86_64-w64-mingw32-ar
LD= @gcc
DRMEM = drmemory.exe -suppress $(LOG_DIR)/custom.txt -batch -logdir

# compiler flags
CFLAGS = -c -pedantic -Wall -Wextra -Werror -I $(INC_DIR)
DBG_CFLAGS = -DDEBUG -g
RLS_CFLAGS = -s -fdata-sections -ffunction-sections -O3

LDLIBS =
LDFLAGS = 
TESTLIB = $(LDLIBS)
TESTLIB += -lcheck

C_OUT = -o

RED := "\033[0;91m"	
LBLUE := "\033[0;94m"	
YELLOW := "\033[0;33m"
RESET := "\033[0m"

define print_info
	@echo -e $(LBLUE)[i] $(1)$(RESET)
endef

define print_warning
	@echo -e $(YELLOW)[!] $(1)$(RESET)	
endef

define print_error
	@echo -e $(RED)[-] $(1)$(RESET)	
endef