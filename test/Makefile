SEABREEZE = ..

export UNAME = $(shell uname)
ifeq ($(UNAME), Darwin)
    APP = seabreeze_test_posix
else ifeq ($(findstring CYGWIN,$(UNAME)), CYGWIN)
    APP = seabreeze_test_windows
else
    APP = seabreeze_test_posix
endif

APPS = $(APP) api_test
OBJS = $(addsuffix .o,$(APPS))
UTIL = spectral_correction.o

all: $(APPS)

include $(SEABREEZE)/common.mk

$(APPS) : $(OBJS) $(UTIL)
	@echo linking $@
	$(CC) -o $@ $@.o $(UTIL) -lseabreeze $(LFLAGS_APP)
