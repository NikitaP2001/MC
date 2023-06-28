MODULE := mc.exe
DEPS:= mc/mc.a test/test.exe

SRC := main.c

include config.mk

export

LDFLAGS += -L $(CUR_BLDIR)/mc
LDLIBS += -l:mc.a

all: CFLAGS += $(DBG_CFLAGS)
all: $(TMPDIRS) project

release: CFLAGS += $(RLS_CFLAGS)
release: LDFLAGS += $(RLS_LDFLAGS)
release: $(TMPDIRS) project

include $(BLDRULES)

TMPDIRS = $(LOGDIR)

$(TMPDIRS):	
	mkdir $@