LIBSRC := fs/fs.a pp/pp.a

SRC = list.c mc.c tools.c

include $(ROOT_DIR)/rules.mk

mc.a: $(LIBDEST)
	@echo AR $@		
	$(AR) rsc -o $@ $^