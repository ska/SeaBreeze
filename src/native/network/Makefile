SEABREEZE = ../../..

all: deps objs

ifeq ($(findstring CYGWIN, $(UNAME)), CYGWIN)
SUBDIRS = windows
else ifeq ($(findstring MINGW, $(UNAME)), MINGW)
SUBDIRS = windows
else
SUBDIRS = posix
endif

include $(SEABREEZE)/common.mk
