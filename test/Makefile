MODULE := test.exe
DEPS := tools/tools.a mc/test.exe

# add definitions to execute tests in subdirectories
TESTS := $(patsubst %/,%,$(dir $(filter %.exe %.a,$(DEPS))))

LDLIBS += -lcheck -l:mc.a -l:tools.a
CFLAGS += -I $(realpath tools) -DTEST_DIRS='"$(TESTS)"'
LDFLAGS += -L $(CUR_BLDIR)/tools

SRC = test.c

include $(BLDRULES)