
# CONFIG TARGET PATH - we need to map the path between sources
# and target modules in the build directory

# from position in source tree identiry corresponding position in
# build directories tree. 
CUR_BLDIR := $(patsubst $(ROOTDIR)%,$(BLDIR)%,$(realpath .))

# add definitions to execute tests in subdirectories
TDIRS := $(patsubst %/,%,$(dir $(filter %.exe %.a,$(DEPS))))
ifneq ($(TDIRS),)
	TESTS := -DTEST_DIRS='"$(TDIRS)"'
else
	TESTS :=
endif

# Objests in build directories tree from sources in source tree
OBJ := $(SRC:.c=.o)
OBJ := $(foreach file, $(OBJ), $(CUR_BLDIR)/$(file))

# deps - modules in subdirectories of current module, we 
# do not directly build it - it does responsible Makefile
BLDEPS := $(foreach dep, $(DEPS), $(CUR_BLDIR)/$(dep))
unexport DEPS

# module - current module/s in current directory we are building now
BLDMOD := $(foreach mod, $(MODULE), $(CUR_BLDIR)/$(mod))


# BUILD RULES

.PHONY: project
project: $(CUR_BLDIR) $(OBJ) $(BLDMOD)
	$(call print_info, "building dep $@")
	$(foreach dep,$(BLDEPS),$(MAKE) -C $(dir $(call orig_path, $(dep))) --no-print-directory ;)

$(BLDIR)/%.o: $(ROOTDIR)/%.c
	@echo CC $<
	$(CC) $(CFLAGS) $(TESTS) $(C_OUT) $@ $<
	
$(CUR_BLDIR):
	mkdir $@	

define orig_path
	$(patsubst $(BLDIR)/%,$(ROOTDIR)/%,$(1))
endef

	
%.a: $(OBJ) $(BLDEPS)
	@echo AR $@		
	$(AR) -rcT $@ $@ $^
	
%.exe: $(OBJ)
	@echo LD $@
	$(LD) $(C_OUT) $@ $^ $(LDLIBS) $(LDFLAGS) 

.PHONY: clean
clean:
	$(call print_info, "cleaning $(CUR_BLDIR)")
	@$(RM) $(BLDMOD)
	@$(RM) $(OBJ)
	@$(foreach dep, $(DEPS), $(MAKE) -C $(dir $(dep)) clean --no-print-directory; )