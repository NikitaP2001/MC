SRC = $(wildcard *.cpp)

OBJ = $(wildcard $(OBJ_DIR)/*.o)
OBJ = $(filter-out $(OBJ_DIR)/main.o, $(OBJECTS))

TEST = $(SRC:.cpp=.$(EXE_EXT))

all:  $(OBJ) $(SRC)
	@echo LD $(TEST)
	$(CPP) $(LDFLAGS) -I $(INC_DIR) $(C_OUT) $(TEST) $^ $(TESTLIB)
