SRC = $(wildcard *.c)

OBJ = $(wildcard $(OBJ_DIR)/*.o)
OBJ = $(filter-out $(OBJ_DIR)/main.o, $(OBJECTS))

TEST = $(SRC:.c=.$(EXE_EXT))

all: $(OBJ) $(SRC)
	@echo LD $(TEST)
	$(CC) $(LDFLAGS) -I $(INC_DIR) $(C_OUT) $(TEST) $^ $(TESTLIB)
	
.PHONY: runtest
runtest: $(TEST)	
	$(foreach test,$^,./$(test) ;)

.PHONY: clean
clean:
	@$(RM) $(TEST)