SEABREEZE = ../../..

all: deps objs

include $(SEABREEZE)/common.mk

ifeq ($(UNAME), Linux)
    SUBDIRS = linux
else ifeq ($(findstring CYGWIN, $(UNAME)), CYGWIN)
    SUBDIRS = winusb
else ifeq ($(findstring MINGW, $(UNAME)), MINGW)
    SUBDIRS = linux
else
    SUBDIRS = osx
endif
