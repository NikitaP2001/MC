MODULE := test.exe
DEPS := tools/tools.a mc/test.exe

LDLIBS += -lcheck -l:mc.a -l:tools.a
CFLAGS += -I $(realpath tools)
LDFLAGS += -L $(CUR_BLDIR)/tools

SRC = test.c

include $(BLDRULES)