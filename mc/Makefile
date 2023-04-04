TARGET = $(BIN_DIR)/mc.exe
SRC = $(wildcard *.c)

OBJ = $(SRC:%.c=$(OBJ_DIR)/%.o)

$(OBJ_DIR)/%.o: %.c
	@echo CC $<
	$(CC) $(CCFLAGS) $(C_OUT) $@ $<
	
all: $(TARGET)
	
$(TARGET): $(OBJ)
	@echo LD $@
	$(LD) $(LDFLAGS) $(C_OUT) $@ $^ $(LDLIBS)

.PHONY: clean
clean:
	@$(RM) $(TARGET)
	@$(RM) $(OBJ)