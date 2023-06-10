OBJ_DIR = $(realpath obj)
BUILD_DIR = $(realpath build)
ROOT_DIR = $(realpath .)
INC_DIR = $(realpath include)
LOG_DIR = $(realpath logs)
TEST_DIR = $(realpath test)

BUILD_RULES = $(realpath rules.mk)

include config.mk
	
export

all: CCFLAGS += $(DBG_CFLAGS)
all: build	

release: CCFLAGS += $(RLS_CFLAGS)
release: LDFLAGS += $(RLS_LDFLAGS)
release: build	

TMPDIRS = obj logs build

$(TMPDIRS):	
	@mkdir $@        
	
build: mc.a 	

test.exe: 	
	@echo LD $@
	$(LD) $(LDFLAGS) $(C_OUT) $@ $^ $(TESTLIB)
	
mc.a:
	@$(MAKE) -C $(basename $@) $@ --no-print-directory

.PHONY: memtest
memtest:
	$(foreach test,$^, $(DRMEM) $(LOG_DIR) -- $(test) ;)	
	
.PHONY: runtest
runtest:
	$(foreach test,$^, $(test) -p 20 ;)
	
.PHONY: clean
clean:
	$(call print_info,Cleaning solution ...)
	@$(MAKE) -C mc $@ --no-print-directory
	@$(MAKE) -C test $@ --no-print-directory