SEABREEZE = ../..


APPS = seabreeze-util $(basename $(wildcard demo-*.c)) 
OBJS = $(addsuffix .o,$(APPS))
UTIL = util.o

all: $(APPS)

include $(SEABREEZE)/common.mk

$(APPS) : $(OBJS) $(UTIL)
	@echo linking $@
	@$(CC) -o $@ $@.o $(UTIL) -lseabreeze $(LFLAGS_APP) -lpthread
