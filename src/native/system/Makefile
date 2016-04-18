SEABREEZE = ../../..

all: deps objs

ifeq ($(findstring CYGWIN, $(UNAME)), CYGWIN)
SUBDIRS = windows
else
SUBDIRS = posix
endif

include $(SEABREEZE)/common.mk
