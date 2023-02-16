TARGET = mc.exe

EXE_EXT = exe

SRC_DIR = $(realpath mc)
OBJ_DIR = $(realpath obj)
INC_DIR = $(realpath include)
TEST_DIR = $(realpath test)

#vpath %.c SRC_DIR TEST_DIR
#vpath %.h INC_DIR
#vpath %.o OBJ_DIR

# toolchain
CC=@gcc -std=c99
LD=@gcc

# compiler flags
CCFLAGS = -c -I $(INC_DIR)
DBG_CCFLAGS = -DDEBUG -g
RLS_CCFLAGS = -s -fdata-sections -ffunction-sections -O3

LDLIBS = 
LDFLAGS = 
TESTLIB = $(LDLIBS)
TESTLIB += -lgtest -lgtest_main

# misc shortcuts
C_OUT = -o 
	
export
        
SUBDIRS = $(SRC_DIR)

all: CCFLAGS += $(DBG_CCFLAGS)
all: $(TARGET)

release: CCFLAGS += $(RLS_CCFLAGS)
release: LDFLAGS += $(RLS_LDFLAGS)
release: $(TARGET)

$(TARGET): OBJ = $(wildcard $(OBJ_DIR)/*.o)	
$(TARGET): $(SUBDIRS)
	@echo LD $@
	$(LD) $(LDFLAGS) $(C_OUT) $@ $(LDLIBS) $(OBJ)
		
.PHONY: $(SUBDIRS)
$(SUBDIRS):	
	@$(MAKE) -C $@ --no-print-directory

.PHONY: $(TEST_DIR)
$(TEST_DIR): $(SUBDIRS)
	$(MAKE) -C $@

runtest: $(TEST)
	$(foreach test,$^,$(test) ;)

.PHONY: clean
clean:
	rm -f $(OBJ_DIR)/*

