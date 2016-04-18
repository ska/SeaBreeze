CC  = $(CROSS_COMPILE)gcc
CPP = $(CROSS_COMPILE)g++

LFLAGS_LIB  =
LFLAGS_APP  = -L${SEABREEZE}/lib
CFLAGS_BASE = -I${SEABREEZE}/include \
              -c \
              -Wall \
              -Wunused \
              -Wmissing-include-dirs \
              -Werror \
              -O0 \
              -fpic \
              -fno-stack-protector \
              -shared
 

export UNAME = $(shell uname)

# MacOS X configuration
ifeq ($(UNAME), Darwin)
    LIBBASENAME = libseabreeze
    SUFFIX      = dylib
    LFLAGS_APP += -L/usr/lib \
                  -lstdc++
    LFLAGS_LIB += -dynamic \
                  -dynamiclib \
                  -framework Carbon \
                  -framework CoreFoundation \
                  -framework IOKit 
                                    
    CFLAGS_BASE = -I${SEABREEZE}/include \
                  -c \
                  -Wall \
                  -Wunused \
                  -Wmissing-include-dirs \
                  -Werror \
                  -g \
                  -O0 \
                  -fpic \
                  -fno-stack-protector 

# Cygwin-32 configuration 
else ifeq ($(findstring CYGWIN, $(UNAME)), CYGWIN)
    # caller can override this, but this is the current Ocean Optics default
    VISUALSTUDIO_PROJ ?= VisualStudio2010
    LIBBASENAME = SeaBreeze
    SUFFIX      = dll
    CFLAGS_BASE = -I${SEABREEZE}/include \
                  -c \
                  -Wall \
                  -Wunused \
                  -Werror \
                  -ggdb3 \
                  -shared

# MinGW-32 configuration
else ifeq ($(findstring MINGW, $(UNAME)), MINGW)
    SUFFIX      = dll
    LIBBASENAME = libseabreeze
    LFLAGS_APP += -L/local/lib \
                  -lusb0 \
                  -lstdc++ \
                  -lm
    LFLAGS_LIB += -L/local/lib \
                  -lusb0 \
                  -shared

# Linux configuration
else
    SUFFIX      = so
    LIBBASENAME = libseabreeze
    LFLAGS_APP += -L/usr/lib \
                  -lusb \
                  -lstdc++ \
                  -lm
    LFLAGS_LIB += -L/usr/lib \
                  -shared
endif

# enable Logger
CFLAGS_BASE += -DOOI_DEBUG

# these are for the .o files making up libseabreeze
CPPFLAGS     = $(CFLAGS_BASE)
CFLAGS       = $(CFLAGS_BASE) -std=gnu99

	# allow for a 32 bit build
ifdef wordwidth
ifeq ($(wordwidth),32)
	CPPFLAGS += -arch i386
	CFLAGS += -arch i386
	LFLAGS_APP += -arch i386
	LFLAGS_LIB += -arch i386
endif
endif

export LIBNAME=$(LIBBASENAME).$(SUFFIX)

SUFFIXES = .c .cpp .o .d

SRCS_CPP := $(wildcard *.cpp)
DEPS_CPP := $(patsubst %.cpp,%.d,$(SRCS_CPP))
OBJS_CPP := $(patsubst %.cpp,%.o,$(SRCS_CPP))

SRCS_C   := $(wildcard *.c)
DEPS_C   := $(patsubst %.c,%.d,$(SRCS_C))
OBJS_C   := $(patsubst %.c,%.o,$(SRCS_C))

VISUALSTUDIO_PROJECTS = VisualStudio2005 \
                        VisualStudio2010 \
                        VisualStudio2012 \
                        VisualStudio2013

ifneq ($(MAKECMDGOALS),clean)
    -include $(DEPFILES)
endif

deps: ${DEPS_CPP} ${DEPS_C}

%.d: %.cpp
	@echo caching $@
	@${CPP} ${CPPFLAGS} -MM $< | sed "s/$*.o/& $@/g" > $@

%.d: %.c
	@echo caching $@
	@${CC} ${CFLAGS} -MM $< | sed "s/$*.o/& $@/g" > $@

%.o: %.cpp
	@echo building $@
	@${CPP} ${CPPFLAGS} $< -o $@

%.o: %.c
	@echo building $@
	@${CC} ${CFLAGS} $< -o $@

objs: subdirs ${OBJS_CPP} ${OBJS_C}
	@/bin/cp *.o ${SEABREEZE}/lib 1>/dev/null 2>&1 || true

subdirs:
	@if [ "$(SUBDIRS)" ] ; then for d in $(SUBDIRS) ; do $(MAKE) -C $$d || exit ; done ; else true ; fi

clean:
	@echo cleaning $$PWD
	@for d in $(SUBDIRS); do $(MAKE) -C $$d $@ || exit; done
	@$(RM) -f *.d *.o *.obj *.exe *.stackdump lib/* $(APPS)
	@for VS in $(VISUALSTUDIO_PROJECTS) ; \
     do \
        if [ -d os-support/windows/$$VS ] ; \
        then \
            echo cleaning os-support/windows/$$VS ; \
            ( cd os-support/windows/$$VS && $(MAKE) clean ) || exit ; \
        fi ; \
     done
	@if [ -d doc ] ; then ( cd doc && $(RM) -rf man rtf html *.err ) ; fi

new: clean all
