TARGET = test.exe
SRC = $(wildcard *.c)

OBJ = $(wildcard $(OBJ_DIR)/*.o)
TEST_OBJ = $(filter-out $(OBJ_DIR)/main.o, $(OBJ))
TEST_OBJ += $(SRC:%.c=$(OBJ_DIR)/%.o)

$(OBJ_DIR)/%.o: %.c
	@echo CC $<
	$(CC) $(CCFLAGS) $(C_OUT) $@ $<

all: $(TEST_OBJ)
	@echo LD $(TARGET)
	$(LD) $(LDFLAGS) $(C_OUT) $(TARGET) $^ $(TESTLIB)

.PHONY: memtest
memtest: $(TARGET)	
	$(foreach test,$^, $(DRMEM) $(LOG_DIR) -- $(test) ;)	
	
.PHONY: runtest
runtest: $(TARGET)	
	$(foreach test,$^,./$(test) ;)

.PHONY: clean
clean:
	@$(RM) $(TEST)