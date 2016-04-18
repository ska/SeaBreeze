SEABREEZE = .

all: seabreeze

SUBDIRS = src test sample-code

include common.mk


.PHONY: doc doc-init doc-user doc-dev help

help:
	@echo "Supported targets include:"
	@echo
	@echo "  make                   Build library, tests and samples (best with -j)"
	@echo "  make wordwidth=32      Build in 32 bit mode for Xojo"
	@echo "  make clean             Remove most build artifacts"
	@echo "  make distclean         Remove all generated artifacts"
	@echo "  make winclean          Invoke Visual Studio's 'clean' target"
	@echo "  make doc               Generate rendered documentation"
	@echo "  make help              This screen"
	
seabreeze: lib/$(LIBNAME)
	make -C test
	make -C sample-code

lib/libseabreeze.$(SUFFIX): initialize
	$(info flags = $(LFLAGS_LIB))
	$(MAKE) -C src
	$(CPP) $(LFLAGS_LIB) -o $@ lib/*.o  

lib/SeaBreeze.dll: initialize
	$(MAKE) -C os-support/windows/$(VISUALSTUDIO_PROJ)

initialize: 
	mkdir -p $(SEABREEZE)/lib

winclean:
	make -C os-support/windows/$(VISUALSTUDIO_PROJ) clean
	make -C test clean
	make -C sample-code clean

distclean: clean doc-init
	@$(RM) -rf lib \
               sample-code/VisualCppConsoleDemo/src/x64 \
               sample-code/VisualCppConsoleDemo/src/Debug \
               sample-code/VisualCppConsoleDemo/*.suo \
               sample-code/VisualCppConsoleDemo/Debug \
               sample-code/CSharpDemo/bin \
               sample-code/CSharpDemo/obj \
               sample-code/CSharpDemo/*.suo \
               sample-code/CSharpDemo/Setup/Release \
               sample-code/CSharpDemo/Setup/Debug \
               sample-code/c/serial-sts-demo
	@for VS in $(VISUALSTUDIO_PROJECTS) ; \
     do \
        $(RM) -rf os-support/windows/$$VS/*.suo \
                  os-support/windows/$$VS/*.ncb \
                  os-support/windows/$$VS/*.cache \
                  os-support/windows/$$VS/Debug \
                  os-support/windows/$$VS/Release \
                  os-support/windows/$$VS/VSProj/SeaBreeze.vcxproj.user \
                  os-support/windows/$$VS/VSProj/x64 \
                  os-support/windows/$$VS/VSProj/*.bak \
                  os-support/windows/$$VS/x64 \
                  os-support/windows/$$VS/Set*/*.bak \
                  os-support/windows/$$VS/Set*/Debug \
                  os-support/windows/$$VS/Set*/Release ; \
     done 

doc-init:
	@echo expunging old docs...
	@( cd doc && $(RM) -rf html man rtf *.err *.out )

doc-user: 
	@echo $(DOXYGEN_BASE)doxygen is generating the user docs...
	@( cd doc && doxygen Doxyfile > doxygen-user.out && mv rtf/refman.rtf rtf/SeaBreezeWrapper_User_Manual.rtf )

doc-dev: 
	@echo $(DOXYGEN_BASE)doxygen is generating the developer docs...
	@( cd doc && doxygen Doxyfile-dev > doxygen-dev.out && mv rtf/refman.rtf rtf/SeaBreeze_Developer_Manual.rtf )

doc: doc-init doc-user doc-dev
	@echo
	@echo The doxygen rendered documentation can be found in doc/. Please note:
	@echo
	@ls -lah doc/rtf/*Manual.rtf
	@echo

