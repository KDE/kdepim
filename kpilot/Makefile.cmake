# This is a GNU makefile. You need GNU make to process it.
# FreeBSD users should use gmake.
#
#

# Unusual configuration things:
#   CMAKE = path to cmake
#   BUILD_DIR = directory to build things in
#   CMAKE_FLAGS = extra flags to CMake.  These will get set by
#      ./configure, saved to CMakeOptions.txt, and read in below...
#

-include Makefile.cmake.in

BUILD_DIR ?= build-$(shell uname -sr | tr -d [:space:] | tr -Cs a-zA-Z0-9 _ )
# these come from CMakeOptions.txt (from ./configure)
CMAKE_FLAGS ?=
CMAKE ?= cmake

all: build-check
	@cd "$(BUILD_DIR)" && $(MAKE)

check: lib tests
	$(BUILD_DIR)/tests/testconstants
	$(BUILD_DIR)/tests/testcategories --data-dir=tests/data
	$(BUILD_DIR)/tests/testaddresses --data-dir=tests/data
	$(BUILD_DIR)/tests/testdatebook --data-dir=tests/data

install: build-check
	@cd "$(BUILD_DIR)" && $(MAKE) install

uninstall: 
	@cd "$(BUILD_DIR)" && $(MAKE) uninstall

lib: $(BUILD_DIR)/lib/libkpilot.so

$(BUILD_DIR)/lib/libkpilot.so: build-check
	@cd "$(BUILD_DIR)/lib" && $(MAKE)

tests: build-check
	@cd "$(BUILD_DIR)/tests" && $(MAKE)

	
build-check:
	test -d "$(BUILD_DIR)" || mkdir -p "$(BUILD_DIR)"
	test -d "$(BUILD_DIR)"
	test -f "$(BUILD_DIR)/Makefile" || (cd "$(BUILD_DIR)" && $(CMAKE) .. )

messages:
	extractrc `find . -name *.rc` > rc.cc
	extractrc `find . -name *.ui` >> rc.cc
	xgettext -o kpilot.po --keyword=i18n rc.cc `find . -name *.h` `find . -name *.cc` 

clean:
	@rm -rf $(BUILD_DIR)

svnclean:
	@rm -rf `svn status --no-ignore | awk '/^[?I]/{print $2}'`

help:
	@echo "Usage: make ( all | install | uninstall | clean )"
	@echo ""

.PHONY : all check install uninstall lib build-check clean help 

