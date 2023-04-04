SRC_DIR = $(realpath mc)
OBJ_DIR = $(realpath obj)
BIN_DIR = $(realpath bin)
INC_DIR = $(realpath include)
LOG_DIR = $(realpath logs)
TEST_DIR = $(realpath test)

# toolchain
CC= @gcc -std=c99
LD= @gcc
DRMEM = drmemory.exe -suppress $(LOG_DIR)/custom.txt -batch -logdir

# compiler flags
CCFLAGS = -c -pedantic -Wall -Wextra -Werror -I $(INC_DIR)
DBG_CCFLAGS = -DDEBUG -g
RLS_CCFLAGS = -s -fdata-sections -ffunction-sections -O3

LDLIBS =
LDFLAGS = 
TESTLIB = $(LDLIBS)
TESTLIB += -lcheck

# misc shortcuts
C_OUT = -o 
	
export
        
SUBDIRS = $(SRC_DIR) $(TEST_DIR)

TMPDIRS = obj logs bin

all: CCFLAGS += $(DBG_CCFLAGS)
all: $(SUBDIRS)

release: CCFLAGS += $(RLS_CCFLAGS)
release: LDFLAGS += $(RLS_LDFLAGS)
release: $(SUBDIRS)

$(TMPDIRS):	
	@mkdir $@
		
.PHONY: $(SUBDIRS)
$(SUBDIRS):	$(TMPDIRS)
	@$(MAKE) -C $@ --no-print-directory

.PHONY: runtest
runtest: $(TMPDIRS)
	@$(MAKE) -C test $@ --no-print-directory	

.PHONY: memtest
memtest: $(TMPDIRS)
	@$(MAKE) -C test $@ --no-print-directory
	
.PHONY: clean
clean:	
	@$(MAKE) -C test $@ --no-print-directory
