SRC = $(wildcard *.c)

OBJ = $(wildcard $(OBJ_DIR)/*.o)
TEST_OBJ = $(filter-out $(OBJ_DIR)/main.o, $(OBJ))

TEST = $(SRC:.c=.$(EXE_EXT))

all: $(TEST_OBJ) $(SRC)
	@echo LD $(TEST)
	$(CC) $(LDFLAGS) $(DBG_CCFLAGS) -I $(INC_DIR) $(C_OUT) \
	$(TEST) $^ $(TESTLIB)

.PHONY: memtest
memtest: $(TEST)	
	$(foreach test,$^, $(DRMEM) $(LOG_DIR) -- $(test) ;)	
	
.PHONY: runtest
runtest: $(TEST)	
	$(foreach test,$^,./$(test) ;)

.PHONY: clean
clean:
	@$(RM) $(TEST)