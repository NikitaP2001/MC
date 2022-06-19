# detect os and cpu arch
ifeq ($(OS),Windows_NT)
	TARGET = target.exe

	# Windows toolchain
	SHELL=cmd.exe
	AS=@ml64.exe 2>NUL
	CC=@c++
	LD=@c++
	RM=@-del /q 2>NUL

	ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
		ARCH ?= 64
	endif

	ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
		ARCH ?= 64
	else
		ARCH ?= 32
	endif

	# set architecture specific sources path
	ifeq ($(ARCH),64)
		ARCH_SRC_DIR = $(realpath arch)/NT/x86_64/src
		ARCH_INC_DIR = $(realpath arch)/NT/x86_64/inc

	else

	endif

	# Compilation flags: masm and gcc
	CCFLAGS = -c -I $(INC_DIR) -I $(ARCH_INC_DIR)
	DBG_CCFLAGS = -DDEBUG -g
	RLS_CCFLAGS = -s -fdata-sections -ffunction-sections -O3
	ACFLAGS = /c /Cp /I $(ARCH_INC_DIR)
	LDLIBS = 
	LDFLAGS = 
	RLS_LDFLAGS = -Wl,--gc-sections,-s

	C_OUT = -o
	A_OUT = /Fo
	

else

endif

SRC_DIR = $(realpath src)
OBJ_DIR = $(realpath obj)
INC_DIR = $(realpath inc)
TEST_DIR = $(realpath test)

vpath %.cpp SRC_DIR TEST_DIR
vpath %.hpp INC_DIR
vpath %.o OBJ_DIR

CPP_SRC = $(wildcard $(SRC_DIR)/*.cpp)
ARCH_CPP_SRC = $(wildcard $(ARCH_SRC_DIR)/*.cpp)
TEST_SRC = $(wildcard $(TEST_DIR)/*.cpp)
ARCH_ASM_SRC = $(wildcard $(ARCH_SRC_DIR)/*.asm)

OBJECTS = $(subst $(SRC_DIR)/,$(OBJ_DIR)/,$(CPP_SRC:.cpp=.o))
OBJECTS += $(subst $(ARCH_SRC_DIR)/,$(OBJ_DIR)/,$(ARCH_CPP_SRC:.cpp=.o))
OBJECTS += $(subst $(ARCH_SRC_DIR)/,$(OBJ_DIR)/,$(ARCH_ASM_SRC:.asm=.o))
# TEST_OBJ = $(subst $(TEST_DIR)/,$(OBJ_DIR)/,$(TESTS_SRC:.cpp=.o))
TEST_OBJ += $(filter-out $(OBJ_DIR)/main.o, $(OBJECTS))

TEST := $(TEST_SRC:.cpp=.exe)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo CC $<
	$(CC) $(CCFLAGS) $(C_OUT) $@ $<

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp
	@echo CC $<
	$(CC) $(CCFLAGS) $(C_OUT) $@ $<
	
$(OBJ_DIR)/%.o: $(ARCH_SRC_DIR)/%.asm
	@echo as $<
	$(AS) $(ACFLAGS) $(A_OUT) $@ $<

$(OBJ_DIR)/%.o: $(ARCH_SRC_DIR)/%.cpp
	@echo CC $<
	$(CC) $(CCFLAGS) $(C_OUT) $@ $<
	
all: CCFLAGS += $(DBG_CCFLAGS)
all: $(TARGET) 

release: CCFLAGS += $(RLS_CCFLAGS)
release: LDFLAGS += $(RLS_LDFLAGS)
release: $(TARGET)

$(TARGET): $(OBJECTS)
	@echo LD $@
	$(LD) $(LDFLAGS) $(C_OUT) $@ $(LDLIBS) $^

TESTLIB = $(LDLIBS)
TESTLIB += -lgtest -lgtest_main

.PHONY: test
test: $(TEST_OBJ) $(TEST_SRC)
	@echo LD $(TEST)
	$(LD) $(LDFLAGS) -I $(INC_DIR) $(C_OUT) $(TEST) $^ $(TESTLIB)


runtest: $(TEST)
	$(foreach test,$^,$(test) ;)

.PHONY: clean
clean:
	$(RM) $(subst /,\,$(OBJ_DIR))
	$(RM) $(subst /,\,$(TEST))

