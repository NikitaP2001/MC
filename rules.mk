
OBJ := $(patsubst $(ROOT_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC))

CUR_BUILDDIR := $(patsubst $(ROOT_DIR)/%,$(BUILD_DIR)/%,$(realpath .))

LIBDEST := $(foreach lib, $(LIBSRC), $(realpath .)/$(lib))
LIBDEST := $(patsubst $(ROOT_DIR)/%.a,$(BUILD_DIR)/%.a,$(LIBDEST))

$(BUILD_DIR)/%.o: $(ROOT_DIR)/%.c $(CUR_BUILDDIR)
	@echo CC $<
	$(CC) $(CFLAGS) $(C_OUT) $@ $<

define orig_path
	$(patsubst $(BUILD_DIR)/%.a,$(ROOT_DIR)/%.a,$(1))
endef

$(CUR_BUILDDIR):
	@mkdir $@ 	
	
$(LIBDEST):	
	$(MAKE) -C $(dir $(call orig_path, $@))	
	
%.a: $(OBJ) $(CUR_BUILDDIR)
	@echo AR $@		
	$(AR) rsc -o $@ $^
	
	
.PHONY: clean
clean:
	@$(RM) $(LIBDEST)	
	@$(RM) $(OBJ)	
	
define add_mc
	src-mc += $(foreach file,$(1),$(realpath $(file)))
endef

define add_test
	src-test += $(foreach file,$(1),$(realpath $(file)))
endef