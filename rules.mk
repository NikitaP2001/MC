
# CONFIG TARGET PATH - we need to map the path between sources
# and target modules in the build directory

# from position in source tree identiry corresponding position in
# build directories tree. 
CUR_BLDIR := $(patsubst $(ROOTDIR)%,$(BLDIR)%,$(realpath .))

# add definitions to execute tests in subdirectories
TDIRS := $(patsubst %/,%,$(dir $(filter %.exe,$(DEPS))))
ifneq ($(TDIRS),)
	TESTS := -DTEST_DIRS='"$(TDIRS)"'
else
	TESTS :=
endif

# Objests in build directories tree from sources in source tree
CSRC := $(filter %.c,$(SRC))
CXXSRC := $(filter %.cxx,$(SRC))
OBJ := $(CSRC:.c=.o)
OBJ += $(CXXSRC:.cxx=.o)
OBJ := $(foreach file, $(OBJ), $(CUR_BLDIR)/$(file))

# deps - modules in subdirectories of current module, we 
# do not directly build it - it does responsible Makefile
BLDEPS := $(foreach dep, $(DEPS), $(CUR_BLDIR)/$(dep))
unexport DEPS

# module - current module/s in current directory we are building now
BLDMOD := $(foreach mod, $(MODULE), $(CUR_BLDIR)/$(mod))


# BUILD RULES

define make_dep
	$(MAKE) -C $(dir $(call orig_path, $(1))) --no-print-directory;
endef

.PHONY: project
project: pr_deps $(OBJ) $(BLDMOD) 
	@:

.PHONY: pr_deps
pr_deps: $(CUR_BLDIR)
	@$(foreach dep,$(BLDEPS),$(call make_dep,$(dep)))

$(BLDIR)/%.o: $(ROOTDIR)/%.c
	@echo CC $<
	$(CC) $(CFLAGS) $(TESTS) $(C_OUT) $@ $<
	
$(BLDIR)/%.o: $(ROOTDIR)/%.cxx
	@echo CC $<
	$(CCX) $(CFLAGS) $(TESTS) $(C_OUT) $@ $<
		
		
$(BLDIR)/%.o: $(ROOTDIR)/%.c
	@echo CC $<
	$(CC) $(CFLAGS) $(TESTS) $(C_OUT) $@ $<
	
$(CUR_BLDIR):
	$(MKDIR) $@	

define orig_path
	$(patsubst $(BLDIR)/%,$(ROOTDIR)/%,$(1))
endef

$(BLDEPS):
	@$(call make_dep,$@)
	
%.a: $(OBJ) $(BLDEPS)
	@echo AR $@		
	@$(RM) $@
	$(AR) -rcs --thin $@ $^
	

BLDEXE := $(filter %.exe,$(BLDMOD))

.PHONY: $(BLDEXE)
$(BLDEXE):
	@echo LD $@
	$(LD) $(C_OUT) $@ $(OBJ) $(LDLIBS) $(LDFLAGS) 

.PHONY: clean
clean:
	$(call print_info, "cleaning $(CUR_BLDIR)")
	@$(RM) $(BLDMOD)
	@$(RM) $(OBJ)
	@$(foreach dep, $(DEPS), $(MAKE) -C $(dir $(dep)) clean --no-print-directory; )