MODULE := mc.exe
DEPS:= mc/ test/

include config.mk
export

SRC := main.c

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

