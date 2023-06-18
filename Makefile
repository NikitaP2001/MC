DEPS:= mc/mc.a test/test.exe

include config.mk

export

LDFLAGS += -L $(CUR_BLDIR)/mc

all: CFLAGS += $(DBG_CFLAGS)
all: $(TMPDIRS) project

release: CFLAGS += $(RLS_CFLAGS)
release: LDFLAGS += $(RLS_LDFLAGS)
release: $(TMPDIRS) project

include $(BLDRULES)

TMPDIRS = $(LOGDIR)

$(TMPDIRS):	
	mkdir $@

.PHONY: memtest
memtest:
	$(foreach test,$^, $(DRMEM) $(LOG_DIR) -- $(test) ;)	
	
.PHONY: runtest
runtest:
	$(foreach test,$^, $(test) -p 20 ;)