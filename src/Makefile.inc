UDIR := $($(U)DIR)

DEFS += -DWITH_$(U)

ifneq ($(strip $(wildcard $(UDIR)/include)),)
INCLUDES += -I$(UDIR)/include
endif

ifneq ($(strip $(wildcard $(UDIR)/src/.libs)),)
LIBDIRS += $(UDIR)/src/.libs
endif
ifeq ($(DEBUG),1)
ifneq ($(strip $(wildcard $(UDIR)/build/debug/src/.libs)),)
LIBDIRS += $(UDIR)/build/debug/src/.libs
endif
endif
ifneq ($(strip $(wildcard $(UDIR)/build/release/src/.libs)),)
LIBDIRS += $(UDIR)/build/release/src/.libs
endif
ifneq ($(strip $(wildcard $(UDIR)/build/src/.libs)),)
LIBDIRS += $(UDIR)/build/src/.libs
endif
ifneq ($(strip $(wildcard $(UDIR))),)
LIBDIRS += $(UDIR)
DOS_LIBDIRS := $(DOS_LIBDIRS);$(UDIR)
endif

LDLIBS += -l$(u)


DEFS := $(DEFS)
INCLUDES := $(INCLUDES)
LIBDIRS := $(LIBDIRS)
LDLIBS := $(LDLIBS)

