DEPS:= mc/mc.a

include config.mk

export

all: CCFLAGS += $(DBG_CFLAGS)
all: $(TMPDIRS) project

release: CCFLAGS += $(RLS_CFLAGS)
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