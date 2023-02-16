SRC = $(wildcard *.c)

OBJ = $(SRC:%.c=$(OBJ_DIR)/%.o)

$(OBJ_DIR)/%.o: %.c
	@echo CC $<
	$(CC) $(CCFLAGS) $(C_OUT) $@ $<

all: $(OBJ)