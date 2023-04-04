TARGET = $(BIN_DIR)/test.exe
SRC = $(wildcard *.c)

OBJ = $(wildcard $(OBJ_DIR)/*.o)
OBJ := $(filter-out $(OBJ_DIR)/main.o, $(OBJ))
OBJ += $(SRC:%.c=$(OBJ_DIR)/%.o)

$(OBJ_DIR)/%.o: %.c
	@echo CC $<
	$(CC) $(CCFLAGS) $(C_OUT) $@ $<

all: $(TARGET)
	
$(TARGET): $(OBJ)
	@echo LD $@
	$(LD) $(LDFLAGS) $(C_OUT) $@ $^ $(TESTLIB)

.PHONY: memtest
memtest: $(TARGET)	
	$(foreach test,$^, $(DRMEM) $(LOG_DIR) -- $(test) ;)	
	
.PHONY: runtest
runtest: $(TARGET)	
	$(foreach test,$^, $(test) ;)

.PHONY: clean
clean:
	@$(RM) $(TARGET)
	@$(RM) $(OBJ)