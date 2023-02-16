SRC = $(wildcard *.c)

OBJ = $(wildcard $(OBJ_DIR)/*.o)
OBJ = $(filter-out $(OBJ_DIR)/main.o, $(OBJECTS))

TEST = $(SRC:.c=.$(EXE_EXT))

all:  $(OBJ) $(SRC)
	@echo LD $(TEST)
	$(LD) $(LDFLAGS) $(C_OUT) $(TEST) $^ $(TESTLIB)
