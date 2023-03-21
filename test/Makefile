SRC = $(wildcard *.c)

OBJ = $(wildcard $(OBJ_DIR)/*.o)
TEST_OBJ = $(filter-out $(OBJ_DIR)/main.o, $(OBJ))
TEST_OBJ += $(SRC:%.c=$(OBJ_DIR)/%.o)
TEST = $(SRC:.c=.$(EXE_EXT))

$(OBJ_DIR)/%.o: %.c
	@echo CC $<
	$(CC) $(CCFLAGS) $(C_OUT) $@ $<

all: $(TEST_OBJ)
	@echo LD $(TEST)
	$(LD) $(LDFLAGS) $(C_OUT) $(TEST) $^ $(TESTLIB)

.PHONY: memtest
memtest: $(TEST)	
	$(foreach test,$^, $(DRMEM) $(LOG_DIR) -- $(test) ;)	
	
.PHONY: runtest
runtest: $(TEST)	
	$(foreach test,$^,./$(test) ;)

.PHONY: clean
clean:
	@$(RM) $(TEST)