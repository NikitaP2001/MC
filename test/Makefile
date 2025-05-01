MODULE :=
DEPS := lib/common.a fs/test.exe pp/test.exe \
		token/test.exe parser/test.exe ir/test.exe 

SRC :=
CFLAGS += -I $(realpath .)/include -Wno-multichar
LDFLAGS += -L $(CUR_BLDIR)/lib

LDLIBS += -l:common.a

include $(BLDRULES)
