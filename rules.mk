
# CONFIG TARGET PATH - we need to map the path between sources
# and target modules in the build directory

# from position in source tree identiry corresponding position in
# build directories tree. 
CUR_BLDIR := $(patsubst $(ROOTDIR)%,$(BLDIR)%,$(realpath .))

# Objests in build directories tree from sources in source tree
OBJ := $(SRC:.c=.o)
OBJ := $(foreach file, $(OBJ), $(CUR_BLDIR)/$(file))
.SECONDARY: $(OBJ)

# deps - modules in subdirectories of current module, we 
# do not directly build it - it does responsible Makefile
BLDEPS := $(foreach dep, $(DEPS), $(CUR_BLDIR)/$(dep))
unexport DEPS

# module - current module/s in current directory we are building now
BLDMOD := $(foreach mod, $(MODULE), $(CUR_BLDIR)/$(mod))


# BUILD RULES

.PHONY: project
project: $(CUR_BLDIR) $(BLDEPS) $(BLDMOD)

$(BLDIR)/%.o: $(ROOTDIR)/%.c
	@echo CC $<
	$(CC) $(CFLAGS) $(C_OUT) $@ $<

$(CUR_BLDIR):
	mkdir $@
	
define orig_path
	$(patsubst $(BLDIR)/%.a,$(ROOTDIR)/%.a,$(1))
endef	
	
$(BLDEPS):
	echo $(CUR_BLDIR)
	$(MAKE) -C $(dir $(call orig_path, $@))	
	
%.a: $(OBJ)
	@echo AR $@		
	$(AR) rsc -o $@ $^
	
%.exe: $(OBJ)
	@echo LD $@
	$(LD) $(LDFLAGS) $(C_OUT) $@ $^ $(LDLIBS)

.PHONY: clean
clean:
	$(call print_info, "cleaning $(CUR_BLDIR)")
	@$(RM) $(BLDMOD)
	@$(RM) $(OBJ)
	@$(foreach dep, $(DEPS), $(MAKE) -C $(dir $(dep)) clean --no-print-directory; )