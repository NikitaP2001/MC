TARGET = mc.exe

EXE_EXT = exe

SRC_DIR = $(realpath mc)
OBJ_DIR = $(realpath obj)
INC_DIR = $(realpath include)
LOG_DIR = $(realpath logs)
TEST_DIR = $(realpath test)

#vpath %.c SRC_DIR TEST_DIR
#vpath %.h INC_DIR
#vpath %.o OBJ_DIR

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
        
SUBDIRS = $(SRC_DIR)

TMPDIRS = obj logs

all: CCFLAGS += $(DBG_CCFLAGS)
all: $(TARGET)

release: CCFLAGS += $(RLS_CCFLAGS)
release: LDFLAGS += $(RLS_LDFLAGS)
release: $(TARGET)

$(TARGET): OBJ = $(wildcard $(OBJ_DIR)/*.o)	
$(TARGET): $(SUBDIRS)
	@echo LD $@
	$(LD) $(LDFLAGS) $(C_OUT) $@ $(LDLIBS) $(OBJ)

$(TMPDIRS):	
	@mkdir $@
		
.PHONY: $(SUBDIRS)
$(SUBDIRS):	$(TMPDIRS)
	@$(MAKE) -C $@ --no-print-directory
	
.PHONY: test
test: $(SUBDIRS)
	@$(MAKE) -C $@ --no-print-directory	

.PHONY: runtest
runtest: $(TMPDIRS)
	@$(MAKE) -C test $@ --no-print-directory	

.PHONY: memtest
memtest: $(TMPDIRS)
	@$(MAKE) -C test $@ --no-print-directory
	
.PHONY: clean
clean: OBJ = $(wildcard $(OBJ_DIR)/*.o)	
clean:	
	@$(MAKE) -C test $@ --no-print-directory
	@$(RM) $(OBJ)
