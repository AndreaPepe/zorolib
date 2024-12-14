# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2024
# Author: Andrea Pepe <pepe.andmj@gmail.com>
#
# Version 0.1.0 (Nov 10th, 2024)
#
# "One to build them all!"
#
# This Makefile is meant to be a 'generic' Makefile, useful to build
# simple applications, but also shared and static libraries.
# Main features are:
# - support for C and C++ code (C++ is partially tested though)
# - automatic header files dependency generation
# - support for building static or dynamic executables
# - support for building shared and static libraries
# - support for customize 'visibility' in libraries
# - special 'install' target to install binaries and header files
# - special 'bin-pkg' target to create a <target>-bin.tar.gz packet with
#   your executable file or shared library
# - special 'dev-pkg' target to create a <target>-dev.tar.gz packet with
#   your libraries and header files
# - special 'pkg' target to create a <target>.tar.gz packet with both
#   'bin' and 'dev' files
#
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License version 3
# along with this program.  If not, see <http://www.gnu.org/licenses/>.


##############################################################################
############################ Basic configuration #############################
##############################################################################

SHELL:=/bin/bash

# To customize the behaviour of this Makefile you have two options (non mutual
# exclusive):
# - edit this file
# - create a config.mk file that overrides what you want to personalize
# In the latter case you can have multiple config files and switch between
# them defining an enviroment variable. For example:
# 	BUILDALT=myalt1 make 		# to build using config-myalt1.mk
# 	BUILDALT=myalt2 make 		# to build using config-myalt2.mk
# 	make 						# to build using config.mk
# Note that, while one might not have a config.mk, when the BUILDALT variable
# is defined the file config-$(BUILDALT).mk must exist
ifneq ($(BUILDALT),)
include config-$(BUILDALT).mk
else
-include config.mk
endif

# Please set your own target name.
# Note that when building libraries the final library name will be:
# - lib$(TARGETNAME).so for shared libraries
# - lib$(TARGETNAME).a for static libraries
TARGETNAME?=a.out

# Target type can be:
# - 'exec' for dynamic executable
# - 'staticexec' for static executable
# - 'lib' to build as library (both shared and static)
# - 'sharedlib' for shared library
# - 'staticlib' for static library
TARGETTYPE?=exec

# Define debug features (multiple flags are allowed)
#  n -> all debug features disabled
#  y -> basic debug flags
#  a -> ASAN support
#  t -> TSAN support -- ignored if 'a' is given
#  s -> GCC static analyzer (since GCC v10)
#  Y -> same as 'ys'
#  A -> same as 'ysa'
#  T -> same as 'yst'
DEBUG?=n

# 'y' -> if you want this Makefile to use 'sudo' when installing target or
# 	 creating a package
# 'n' -> if you want to type 'sudo' yourself whenever you think you need to.
USESUDO?=n

# 'y' -> if you want to run GCC inside GDB to debug compilation process
# 'n' -> otherwise (std. behaviour)
USEGDB?=n

# Change the build output directory (default is .)
BUILD_OUTPUT?=

# Add here extra include directories (EXTRA_DIRS are automatically added)
#INCFLAGS?=-I../your_include_directory

# Add here your -L and -l linker options
# Remember to add -lstdc++ when linking C++ code
LDFLAGS?=


# Add here the static libraries (*.a files) you want to link
# 
# NOTE:
# The GNU ld linker is a so-called smart linker. It will keep track of the
# functions used by preceding static libraries, permanently tossing out those
# functions that are not used from its lookup tables. The result is that if you
# link a static library too early, then the functions in that library are no
# longer available to static libraries later on the link line.
# The typical UNIX linker works from left to right, so put all your dependent
# libraries on the left, and the ones that satisfy those dependencies on the
# right of the link line. You may find that some libraries depend on others
# while at the same time other libraries depend on them. This is where it gets
# complicated. When it comes to circular references, fix your code!
# For more info: http://stackoverflow.com/questions/45135/why-does-the-order-in-which-libraries-are-linked-sometimes-cause-errors-in-gcc
STATICLIBS?=

# file name for version file
VERSIONFILE?=VERSION

# Author name
AUTHOR?=

# Default verbose configuration
# 0 -> pretty output
# 1 -> verbose output (build and clean commands shown)
# 2 -> very verbose output (all commands are shown)
DEFAULT_VERBOSE?=0


##############################################################################
############################## Advanced tweaks ###############################
##############################################################################

# When building libraries set this variable to 'y' to manually select which
# function will be available through the library. If you choose to do this,
# remember to mark "__public" all the functions you want to export.
# For example:
#                 int __public mypublicfunc(void) { ... }
#
OPTIMIZE_LIB_VISIBILITY?=n

# Use this feature if you need to load a script that changes the environment
# for all the commands executed in this makefile
# i.e.: to me it is useful when I'm cross compiling to add the cross tools
#       to the PATH and LD_LIBRARY_PATH.
ENV_SCRIPT?=

ENV_SCRIPT_OUTPUT=/tmp/makeenv.$(TARGETNAME).$(TARGETTYPE)
ifneq ($(ENV_SCRIPT),)
IGNOREME := $(shell bash -c "source \"$(ENV_SCRIPT)\"; env | sed 's/=/:=/' | sed 's/^/export /' > \"$(ENV_SCRIPT_OUTPUT)\"")
include $(ENV_SCRIPT_OUTPUT)
endif

# Build also sources from all the directories listed in EXTRA_DIRS
# By default, files from all subdirectories are built.
# If you want to build only few directories, just list them in the variable.
# Not to include any directory, just leave the variable empty
ifeq ($(BUILD_OUTPUT),)
EXTRA_DIRS?=$(shell find -L . -mindepth 1 -path ./.git -prune -o \( -type d -a \! -empty \) -print)
SUBTARGETS_DIRS?=$(shell find -L . -mindepth 1 -path ./.git -prune -o \( -type d -a \! -empty \) -print)
else
_IGNORE:=$(shell mkdir -p $(BUILD_OUTPUT))
EXTRA_DIRS?=$(shell find -L . -mindepth 1 -path ./.git -prune -o \( -type d -a \! -empty -a \! -samefile $(BUILD_OUTPUT) \) -print )
SUBTARGETS_DIRS?=$(shell find -L . -mindepth 1 -path ./.git -prune -o \( -type d -a \! -empty -a \! -samefile $(BUILD_OUTPUT) \) -print )
endif

_EXTRA_DIRS:=$(shell for i in $(EXTRA_DIRS) ; do if test ! -r $${i}/Makefile 2>/dev/null ; then echo $${i} ; fi ; done)
_SUBTARGETS_DIRS:=$(shell for i in $(SUBTARGETS_DIRS) ; do if test -r $${i}/Makefile 2>/dev/null ; then echo $${i} ; fi ; done)

# Uncomment (and eventually change the header file name)
# to install an header file along with your target
# (this is very common for libreries)
# It is allowed only one single file with extension .h
# Any file that it includes using doblue quotes (not angle brackets!) is going
# to be installed as well
#INSTALL_HEADER?=$(TARGETNAME).h

# The filesystem which you build against
# Note that in this file system will also be installed header files and libraries.
BUILDFS?=/

# The filesystem where you want your binary files (executables and shared
# libraries) to be installed into
INSTALL_ROOT?=$(BUILDFS)

INSTALL_PREFIX?=/usr/local

# INSTALL_PREFIX's subdirectory where to install targets
# Leave it commented to use defaults:
# - 'lib' (or lib64) for libraries, 'bin' for executables
#INSTALL_DIR?=mydir

#CROSS_COMPILE?=arm-arago-linux-gnueabi-
CROSS_COMPILE?=

ifneq ($(CROSS_COMPILE),)
	LIBSUBDIR?=lib
	CC=$(CROSS_COMPILE)gcc
	CXX=$(CROSS_COMPILE)g++
	CPP=$(CROSS_COMPILE)cpp
	AR=$(CROSS_COMPILE)ar
else
ifeq ($(shell uname -m), x86_64)
	LIBSUBDIR?=lib64
else
	LIBSUBDIR?=lib
endif
	CC?=gcc
	CXX?=g++
	CPP?=cpp
	AR?=ar
endif

CCVERSION=$(shell $(CC) -dumpfullversion -dumpversion | sed 's/\./0/g')

POST_INSTALL_SCRIPT?=./post_install.sh
POST_INSTALL_SCRIPT_CMD?=BUILDFS="$(BUILDFS)" INSTALL_ROOT="$(INSTALL_ROOT)" INSTALL_PREFIX="$(INSTALL_PREFIX)" TARGETNAME="$(TARGETNAME)" $(POST_INSTALL_SCRIPT)

ifeq ($(USESUDO),y)
INSTALL?=sudo install -D
SUDORM?=sudo $(RM)
RUN_POST_INSTALL_SCRIPT?=sudo $(POST_INSTALL_SCRIPT_CMD)
else
INSTALL?=install -D
SUDORM?=$(RM)
RUN_POST_INSTALL_SCRIPT?=$(POST_INSTALL_SCRIPT_CMD)
endif

EXTRA_CFLAGS?=
CFLAGS?=-Wall -Wextra -Wno-unused-parameter -fPIC $(EXTRA_CFLAGS)
CXXFLAGS?=-Wall -Wextra -Wno-unused-parameter -fPIC $(EXTRA_CFLAGS)

_LD_PATH_0:=$(shell echo $(BUILDFS)/$(INSTALL_PREFIX)/$(LIBSUBDIR) | sed 's_/\+_/_g')
_LD_PATH_1:=$(shell echo $(BUILDFS)/usr/$(LIBSUBDIR) | sed 's_/\+_/_g')
LDFLAGS:=$(LDFLAGS) -L"$(_LD_PATH_0)" -L"$(_LD_PATH_1)"

# Defining _DEBUG (DEBUG is propagated, so changing is not a good idea)
ifneq ($(findstring A,$(DEBUG)),)
	_DEBUG=$(DEBUG)ysa
else ifneq ($(findstring T,$(DEBUG)),)
	_DEBUG=$(DEBUG)yst
else ifneq ($(findstring Y,$(DEBUG)),)
	_DEBUG=$(DEBUG)ys
else
	_DEBUG=$(DEBUG)
endif

ifneq ($(findstring y,$(_DEBUG)),)
	# The following flags are needed to support backtrace() function on
	# both x86 and arm.
	CFLAGS+= -g -O0 -rdynamic -fno-omit-frame-pointer -fno-inline	\
					-funwind-tables
	CXXFLAGS+= -g -O0 -rdynamic -fno-omit-frame-pointer -fno-inline	\
					-funwind-tables
	OPTIMIZE_LIB_VISIBILITY=n
	CFLAGS+=-D__public=
	CXXFLAGS+=-D__public=
else
	CFLAGS+= -O3 -DNDEBUG
	CXXFLAGS+= -O3 -DNDEBUG
endif

ifneq ($(findstring a,$(_DEBUG)),)
	CFLAGS+=-fsanitize=address
	CXXFLAGS+=-fsanitize=address
	LDFLAGS+=-fsanitize=address
else ifneq ($(findstring t,$(_DEBUG)),)
	CFLAGS+=-fsanitize=thread
	CXXFLAGS+=-fsanitize=thread
	LDFLAGS+=-fsanitize=thread
endif

ifneq ($(findstring s,$(_DEBUG)),)
	# -fanalyzer option is actually supported in GCC suite only
	ifneq (,$(filter $(shell echo $(CC) | cut -c1-3),cc gcc))
		ifeq (g++,$(shell echo $(CXX) | cut -c1-3))
			SUPPORT_FANALYZER:=$(shell expr $(CCVERSION) \>= 100000)
			ifeq ($(SUPPORT_FANALYZER),1)
				CFLAGS+=-fanalyzer -DSUPPORT_FANALYZER
				CXXFLAGS+=-fanalyzer -DSUPPORT_FANALYZER
			endif
		endif
	endif
endif

ifeq ($(USEGDB),y)
GDBFLAGS=-wrapper gdb,--args
else
GDBFLAGS=
endif


##############################################################################
####### NOTE! You should not need to change anything below this line! ########
##############################################################################

# Find sources and construct output filepaths
CSRC:=$(shell for i in $(_EXTRA_DIRS) ; do ls $${i}/*.c 2>/dev/null ; done)
CSRC+=$(shell ls *.c 2>/dev/null)
ASRC:=$(shell for i in $(_EXTRA_DIRS) ; do ls $${i}/*.S 2>/dev/null ; done)
ASRC+=$(shell ls *.S 2>/dev/null)
CPPSRC1:=$(shell for i in $(_EXTRA_DIRS) ; do ls $${i}/*.cpp 2>/dev/null ; done)
CPPSRC1+=$(shell ls *.cpp 2>/dev/null)
CPPSRC2:=$(shell for i in $(_EXTRA_DIRS) ; do ls $${i}/*.cc 2>/dev/null ; done)
CPPSRC2+=$(shell ls *.cc 2>/dev/null)
CPPSRC3:=$(shell for i in $(_EXTRA_DIRS) ; do ls $${i}/*.C 2>/dev/null ; done)
CPPSRC3+=$(shell ls *.C 2>/dev/null)
CPPSRC:=$(CPPSRC1) $(CPPSRC2) $(CPPSRC3)
SRC:=$(CSRC) $(CPPSRC) $(ASRC)
COBJ:=$(CSRC:.c=.o)
AOBJ:=$(ASRC:.S=.o)
CPPOBJ:=$(CPPSRC1:.cpp=.o)
CPPOBJ+=$(CPPSRC2:.cc=.o)
CPPOBJ+=$(CPPSRC3:.C=.o)
OBJ:=$(COBJ) $(CPPOBJ) $(AOBJ)
DEP:=$(COBJ:.o=.d)
DEP+=$(CPPSRC1:.cpp=.dd1)
DEP+=$(CPPSRC2:.cc=.dd2)
DEP+=$(CPPSRC3:.C=.dd3)

# Build output directory if needed
ifneq ($(BUILD_OUTPUT),)
_CREATE_BUILD_OUTPUT:=$(shell mkdir -p $(BUILD_OUTPUT))
_CREATE_BUILD_OUTPUT:=$(shell for i in $(_EXTRA_DIRS) ; do mkdir -p $(BUILD_OUTPUT)/$${i} ; done)
# From now on, making sure there is a slash at the end
BUILD_OUTPUT:=$(BUILD_OUTPUT:%/=%)/
OBJ:=$(OBJ:%=$(BUILD_OUTPUT)%)
DEP:=$(DEP:%=$(BUILD_OUTPUT)%)
endif

.SECONDARY: $(DEP) $(OBJ)

INCFLAGS+=$(shell for i in $(_EXTRA_DIRS) ; do echo "-I$${i} " ; done)

# Please note that the order of "-I" directives is important. My choice is to
# first look for headers in the sources, and than in the system directories.
_I_FLAG_0:=$(shell echo $(BUILDFS)/$(INSTALL_PREFIX)/include | sed 's_/\+_/_g')
_I_FLAG_1:=$(shell echo $(BUILDFS)/usr/include | sed 's_/\+_/_g')
INCFLAGS:=-I. $(INCFLAGS) -I"$(_I_FLAG_0)" -I"$(_I_FLAG_1)"

CFLAGS+=$(INCFLAGS)
CXXFLAGS+=$(INCFLAGS)

LINK?=$(CC)

TARGET=$(TARGETNAME)

ifeq ($(TARGETTYPE),exec)
	TARGETEXT=$(TARGETNAME)
	INSTALL_DIR?=bin
	OPTIMIZE_LIB_VISIBILITY=n
endif

ifeq ($(TARGETTYPE),staticexec)
	TARGETEXT=$(TARGETNAME)
	LINK+=-static
	INSTALL_DIR?=bin
	OPTIMIZE_LIB_VISIBILITY=n
endif

ifeq ($(TARGETTYPE),sharedlib)
	TARGET=lib$(TARGETNAME).so
	TARGETEXT=$(TARGETNAME)lib
	LDFLAGS+=-Wl,-soname,$(TARGET) -shared
	INSTALL_DIR?=$(LIBSUBDIR)
endif

ifeq ($(TARGETTYPE),lib)
	# Same as sharedlib!
	TARGET=lib$(TARGETNAME).so
	TARGETEXT=$(TARGETNAME)lib
	LDFLAGS+=-Wl,-soname,$(TARGET) -shared
	INSTALL_DIR?=$(LIBSUBDIR)
endif

ifeq ($(TARGETTYPE),staticlib)
	TARGET=lib$(TARGETNAME).a
	TARGETEXT=$(TARGETNAME)lib
	INSTALL_DIR?=$(LIBSUBDIR)
endif

# Checking if we are in a git repository or not; if so use git tag to compute
# the target version, otherwise version will be "latest"
USINGGIT?=$(shell if git remote show -n &>/dev/null ;then echo -n y ; fi)
ifeq ($(USINGGIT),y)
	COMMIT=$(shell git rev-parse HEAD)
	DATE?=$(shell git show -s --format=%ci HEAD | cut -d ' ' -f 1)
	USINGGIT=$(shell if git describe --abbrev=0 --tags &>/dev/null ; then echo -n y ; else echo -n n ; fi)
endif
ifeq ($(USINGGIT),y)
	LATEST_TAG=$(shell git describe --abbrev=0 --tags)
	COUNT_EXTRA_COMMITS=$(shell git rev-list $(LATEST_TAG)..HEAD --count)
	VERSION?=$(LATEST_TAG).$(COUNT_EXTRA_COMMITS)
else
	COMMIT?='Not in a git repository'
	DATE?=$(shell date +%s)
	VERSION?=latest
endif

# Output package files
TMPDIR=$(shell readlink -mn $(BUILD_OUTPUT)._tmp)
PKG?=$(BUILD_OUTPUT)$(TARGETEXT).tar.gz
PKG:=$(shell readlink -mn $(PKG))
BINPKG?=$(BUILD_OUTPUT)$(TARGETEXT)-bin.tar.gz
BINPKG:=$(shell readlink -mn $(BINPKG))
DEVPKG?=$(BUILD_OUTPUT)$(TARGETEXT)-dev.tar.gz
DEVPKG:=$(shell readlink -mn $(DEVPKG))
SRCPKGDIR?=$(TARGETEXT)-src
SRCPKG?=$(BUILD_OUTPUT)$(TARGETEXT)-src-$(VERSION).tar.gz
SRCPKG:=$(shell readlink -mn $(SRCPKG))
BAKPKGDIR?=$(TARGETEXT)-backup
BAKPKG?=$(BUILD_OUTPUT)$(TARGETEXT)-backup-$(VERSION).tar.gz
BAKPKG:=$(shell readlink -mn $(BAKPKG))

ifneq ($(INSTALL_HEADER),)
	INSTALL_HEADER_DIR=$(shell dirname $(abspath $(INSTALL_HEADER)))
	# Note that here I use := instead of = because I want CFLAGS to expand
	# immediately (before including $(VISHEADER))
	____MAIN_HEADER_DEP:=$(shell PATH="$(PATH)" $(CPP) $(CFLAGS) $(CXXFLAGS) -MM $(INSTALL_HEADER) | sed 's,\($*\)\.o[ :]*,\1.h: ,g' | sed 's,\\,,g')
	# Removing target
	___MAIN_HEADER_DEP:=$(shell echo $(____MAIN_HEADER_DEP) | cut -d ':' -f 2)
	
	# Work with absolute paths and remove the headers that are not
	# contained under the directory of the specified install header
	__MAIN_HEADER_DEP:=$(abspath $(___MAIN_HEADER_DEP))
	_MAIN_HEADER_DEP:=$(foreach h, $(__MAIN_HEADER_DEP), $(filter $(INSTALL_HEADER_DIR)%, $(h)))
	MAIN_HEADER_DEP:="$(INSTALL_HEADER): $(_MAIN_HEADER_DEP)"
	HEADERS_INSTALL_DIR=$(BUILDFS)/$(INSTALL_PREFIX)/include
endif

ifeq ($(OPTIMIZE_LIB_VISIBILITY),y)
	VISHEADER=$(BUILD_OUTPUT)__vis.h
	CFLAGS+=-fvisibility=hidden -include $(VISHEADER)
	CXXFLAGS+=-fvisibility=hidden -include $(VISHEADER)
else
	VISHEADER=
endif

INSTALL_TARGET=$(shell echo $(INSTALL_ROOT)/$(INSTALL_PREFIX)/$(INSTALL_DIR)/$(shell basename $(TARGET)) | sed 's_/\+_/_g')
BUILDFS_TARGET=$(shell echo $(BUILDFS)/$(INSTALL_PREFIX)/$(INSTALL_DIR)/$(shell basename $(TARGET)) | sed 's_/\+_/_g')

CTAGS=$(shell which ctags 2>/dev/null)

ifneq ($(CTAGS),)
	CTAGSTARGET=$(BUILD_OUTPUT)tags
else
	CTAGSTARGET=
endif

TARGET:=$(BUILD_OUTPUT)$(TARGET)

ifeq ($(TARGETTYPE),lib)
	ALLTARGETS=$(TARGET) $(TARGET:.so=.a)
else
	ALLTARGETS=$(TARGET)
endif

INSTALLTARGETS=$(ALLTARGETS)

ifneq ($(INSTALL_HEADER),)
	INSTALL_HEADER_DIR=$(shell dirname $(abspath $(INSTALL_HEADER)))
	REMOVE_HEADER_DIR:=$(shell for i in $(_EXTRA_DIRS) ; do		      \
		if [ "$$(cd $$i && pwd -P)" == "$(INSTALL_HEADER_DIR)" ];then \
			echo -n "y" ; fi ; done)
	ifneq ($(REMOVE_HEADER_DIR),y)
		TARGET_HEADERS:=$(HEADERS_INSTALL_DIR)/$(shell basename $(INSTALL_HEADER))
	else
		TARGET_HEADERS:=$(HEADERS_INSTALL_DIR)/$(INSTALL_HEADER)
	endif
endif

# Beautify output (unless 'make V=1' is given)
# Inspired by the Linux kernel Makefile
ifeq ("$(origin V)", "command line")
	BUILD_VERBOSE = $(V)
endif

ifndef BUILD_VERBOSE
	BUILD_VERBOSE = $(DEFAULT_VERBOSE)
endif

ifneq ($(BUILD_VERBOSE),0)
define pretty_print
endef
RM = rm -v
RMDIR = rmdir -v
ifeq ($(BUILD_VERBOSE),1)
	Q = @
else
	Q =
endif
else
.SILENT:
TABS?=
define pretty_print
	printf "$(TABS)$(1)\n"
endef
RM = rm
RMDIR = rmdir
endif

SUBT_MAKE:=TABS="$(TABS)\t" USESUDO=$(USESUDO) DEBUG=$(DEBUG) $(MAKE)

###############################################################################
########################## Begin of target make rules #########################	
###############################################################################

all: target subtargets

target: $(ALLTARGETS)

subtargets: target
	+@for t in $(_SUBTARGETS_DIRS) ; do			\
		$$__UNDEF__$(call pretty_print,SUBTARGET $$t);	\
		$(SUBT_MAKE) -C $$t || break;			\
	done

$(BUILD_OUTPUT)%.a: $(OBJ)
	$(call pretty_print,AR $@)
	$(AR) -rcs $@ $(OBJ)

$(BUILD_OUTPUT)%.o: %.c
	$(call pretty_print,CC $<)
	$(CC) -c -o $@ $(CPPFLAGS) $(CFLAGS) $(GDBFLAGS) $<

$(BUILD_OUTPUT)%.o: %.S
	$(call pretty_print,AS $<)
	$(CC) -c -o $@ $(CPPFLAGS) $<

ifneq ($(TARGETTYPE),staticlib)
$(TARGET): $(OBJ)
	$(call pretty_print,LD $@)
	$(LINK) $(OBJ) $(STATICLIBS) $(LDFLAGS) -o $@
endif

ifneq ($(MAKECMDGOALS:clean%=CLEAN),CLEAN)
-include $(DEP)
endif

# These few lines were inspired by:
# http://www.makelinux.net/make3/make3-CHP-2-SECT-7
# Note: use -MM instead of -M if you do not want to include system headers in
#       the dependencies
$(BUILD_OUTPUT)%.d: %.c $(VISHEADER)
	$(Q)$(CC) $(CFLAGS) -MM -MT $@ $< > $@.$$$$;		\
	sed 's,\($*\)\.d[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;	\
	rm -f $@.$$$$

$(BUILD_OUTPUT)%.dd1: %.cpp $(VISHEADER)
	$(Q)$(CXX) $(CXXFLAGS) -MM -MT $@ $< > $@.$$$$;		\
	sed 's,\($*\)\.dd1[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;	\
	rm -f $@.$$$$

$(BUILD_OUTPUT)%.dd2: %.cc $(VISHEADER)
	$(Q)$(CXX) $(CXXFLAGS) -MM -MT $@ $< > $@.$$$$;		\
	sed 's,\($*\)\.ddd2[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;	\
	rm -f $@.$$$$

$(BUILD_OUTPUT)%.dd3: %.C $(VISHEADER)
	$(Q)$(CXX) $(CXXFLAGS) -MM -MT $@ $< > $@.$$$$;		\
	sed 's,\($*\)\.dd3[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;	\
	rm -f $@.$$$$

ifeq ($(OPTIMIZE_LIB_VISIBILITY),y)
$(VISHEADER):
	@echo '#ifndef _VIS_H_'				>  $(VISHEADER)
	@echo '#define _VIS_H_'				>> $(VISHEADER)
	@echo '#pragma GCC visibility push(default)'	>> $(VISHEADER)
	@echo '#pragma GCC visibility pop'		>> $(VISHEADER)
	@echo '#define __public __attribute__((visibility ("default")))' \
							>> $(VISHEADER)
	@echo '#endif'					>> $(VISHEADER)
endif

SOURCEFILELIST=._source_file_list

$(SOURCEFILELIST): $(DEP) FORCE
	$(Q)for f in $(shell find . -maxdepth 1 -name \*.d) ; do	\
		cut -d ':' -f 2 $$f | sed 's,\\,\ ,g' | 		\
			sed 's,\ ,\n,g' ; done > $@
ifneq ($(_EXTRA_DIRS),)
	$(Q)for f in $(shell find $(_EXTRA_DIRS) -maxdepth 1 -name \*.d) ; do \
		cut -d ':' -f 2 $$f | sed 's,\\,\ ,g' | 		      \
		sed 's,\ ,\n,g' ; done >> $@
endif
ifeq ($(OPTIMIZE_LIB_VISIBILITY),y)
	$(Q)echo $(VISHEADER) >> $@
endif
	$(Q)for f in $(MAKEFILE_LIST) ; do			\
		if [ "$${f: -2}" != ".d" ];then			\
			echo $$f ; fi ; done >> $@
	$(Q)sort -u -o $@ $@

# Sub targets source file list
SUBT_SOURCEFILELIST=._subt_source_file_list

$(SUBT_SOURCEFILELIST): FORCE
	$(Q)truncate -s 0 "$(SUBT_SOURCEFILELIST)"
	$(Q)for t in $(_SUBTARGETS_DIRS) ; do				\
		$(SUBT_MAKE) -sC $$t $(SOURCEFILELIST) || break ;	\
		for j in $$(cat $$t/$(SOURCEFILELIST)) ; do		\
			realpath -s --relative-base=. "$$t/$$j" ;	\
		done >> $@ ;						\
		rm -f $$t/$(SOURCEFILELIST);				\
	done
	$(Q)sort -u -o $@ $@

# Full source file list
FULL_SOURCEFILELIST=._full_source_file_list

$(FULL_SOURCEFILELIST): $(SOURCEFILELIST) $(SUBT_SOURCEFILELIST)
	$(Q)sort -mu $^ > $@

$(CTAGSTARGET): $(FULL_SOURCEFILELIST)
ifneq ($(CTAGS),)
	$(call pretty_print,CTAGS $@)
	$(CTAGS) -f $@ -L $^
else
	$(call pretty_print,ctags command not found)
endif

# Begin installation targets
post-install-script:
ifneq ($(POST_INSTALL_SCRIPT),)
	@test ! -x $(POST_INSTALL_SCRIPT) ||			\
		$$__UNDEF__$(call pretty_print,RUN $(POST_INSTALL_SCRIPT))
	$(Q)test ! -x $(POST_INSTALL_SCRIPT) || $(RUN_POST_INSTALL_SCRIPT)
	@for t in $(_SUBTARGETS_DIRS) ; do			\
		INSTALL_ROOT="$(INSTALL_ROOT)"			\
		BUILDFS="$(BUILDFS)"				\
			$(SUBT_MAKE) -C $$t $@ || break ;	\
	done
endif

install: $(INSTALLTARGETS) install-bin-pkg install-dev-pkg post-install-script

install-bin install-bin-pkg: $(INSTALLTARGETS)
ifneq ($(TARGETTYPE),staticlib)
	$(call pretty_print,Installing binaries to your root filesystem:)
	$(call pretty_print, * $(TARGET) -> \"$(INSTALL_TARGET)\")
	$(INSTALL) $(TARGET) "$(INSTALL_TARGET)"
endif
	@for t in $(_SUBTARGETS_DIRS) ; do			\
		$$__UNDEF__$(call pretty_print,TARGET $$t);	\
		INSTALL_ROOT="$(INSTALL_ROOT)"			\
		BUILDFS="$(BUILDFS)"				\
			$(SUBT_MAKE) -C $$t $@ || break ;	\
	done

install-dev install-dev-pkg: $(TARGET_HEADERS) $(INSTALLTARGETS)
ifneq ($(findstring lib,$(TARGETTYPE)),)
ifneq ($(INSTALL_ROOT),$(BUILDFS))
	$(call pretty_print,Installing binaries to your build filesystem:)
	$(call pretty_print, * $(TARGET) -> \"$(BUILDFS_TARGET)\")
	$(INSTALL) $(TARGET) "$(BUILDFS_TARGET)"
endif
ifeq ($(TARGETTYPE),lib)
	$(call pretty_print,Installing binaries to your root filesystem:)
	$(call pretty_print, * $(TARGET:.so=.a) -> \"$(BUILDFS_TARGET:.so=.a)\")
	$(INSTALL) $(TARGET:.so=.a) "$(BUILDFS_TARGET:.so=.a)"
endif
endif
	@for t in $(_SUBTARGETS_DIRS) ; do			\
		$$__UNDEF__$(call pretty_print,TARGET $$t);	\
		INSTALL_ROOT="$(INSTALL_ROOT)"			\
		BUILDFS="$(BUILDFS)"				\
			$(SUBT_MAKE) -C $$t $@ || break ;	\
	done

ifneq ($(MAIN_HEADER_DEP),)
$(TARGET_HEADERS): $(_MAIN_HEADER_DEP)
	$(call pretty_print,Installing headers to your build filesystem:)
	@for h in $^ ; do						\
		h_dest=$$(echo "$$h" | sed "s|^$(INSTALL_HEADER_DIR)/||g"); \
		if [ ! -r "$(HEADERS_INSTALL_DIR)/$$h_dest" ]; then \
			$$__UNDEF__$(call pretty_print, * $$h -> \"$(HEADERS_INSTALL_DIR)/$$h_dest\"); \
			$(INSTALL) -m 0644 $$h $(HEADERS_INSTALL_DIR)/$$h_dest;	\
		else 								\
			$$__UNDEF__$(call pretty_print, * $$h -> \"$(HEADERS_INSTALL_DIR)/$$h_dest\" already installed); \
		fi 								\
	done
endif

$(VERSIONFILE):
	@echo "$(TARGETEXT) $(VERSION)" > $@
	@echo "Commit: $(COMMIT)" >> $@
ifneq ($(AUTHOR),)
	@echo "Copyright ${AUTHOR}"  >> $@
endif
	@echo "Release date: $(DATE)" >> $@
	@echo "Package created: $(shell date)" >> $@
	@echo "Version file:"
	@cat $@

$(TMPDIR):
	@$(SUDORM) -rf "$(TMPDIR)"
	@mkdir -p "$@"

pkg: clean-files $(TMPDIR) all
	@INSTALL_ROOT="$(TMPDIR)" BUILDFS="$(TMPDIR)" $(MAKE)	\
			install-bin-pkg install-dev-pkg post-install-script
	@$(MAKE) $(PKG)

bin-pkg: clean-files $(TMPDIR) all
	@INSTALL_ROOT="$(TMPDIR)" $(MAKE) install-bin-pkg post-install-script
	@$(MAKE) $(BINPKG)

dev-pkg: clean-files $(TMPDIR) all
	@BUILDFS="$(TMPDIR)" $(MAKE) install-dev-pkg post-install-script
	@$(MAKE) $(DEVPKG)

backup-pkg: $(TMPDIR)
	@mkdir -p "$(TMPDIR)/$(BAKPKGDIR)"
	@cp -rHv * "$(TMPDIR)/$(BAKPKGDIR)"
	@$(MAKE) -C "$(TMPDIR)/$(BAKPKGDIR)" clean
	@$(MAKE) -C "$(TMPDIR)/$(BAKPKGDIR)" $(VERSIONFILE)
	@$(MAKE) -C "$(TMPDIR)/$(BAKPKGDIR)" clean-files clean-pkg
	@$(MAKE) $(BAKPKG)

src-pkg: $(TMPDIR) $(FULL_SOURCEFILELIST)
	@mkdir -p "$(TMPDIR)/$(SRCPKGDIR)"
	@cp -av LICENSES "$(TMPDIR)/$(SRCPKGDIR)/"
	@for i in $(shell cat $(FULL_SOURCEFILELIST)) ; do		\
		$(INSTALL) -m 0644 $$i "$(TMPDIR)/$(SRCPKGDIR)/$$i";	\
	done
	@$(MAKE) -C "$(TMPDIR)/$(SRCPKGDIR)" clean
	@$(MAKE) -C "$(TMPDIR)/$(SRCPKGDIR)" $(VERSIONFILE)
	@$(MAKE) -C "$(TMPDIR)/$(SRCPKGDIR)" clean-files clean-pkg
	@$(MAKE) $(SRCPKG)

$(PKG) $(BINPKG) $(DEVPKG) $(SRCPKG) $(BAKPKG): FORCE
	@if rmdir "$(TMPDIR)" &>/dev/null ;then echo "Nothing to pack" && \
								exit 1; fi
	$(Q)cd "$(TMPDIR)" && tar -c --sort=name --owner=root:0 --group=root:0 \
			      --mtime="1980-01-01 00:00:00" -z -v -f $@        \
			      --exclude=.gitignore *
	$(Q)$(SUDORM) -rf "$(TMPDIR)"
	@echo ""
	@echo "Package $@ built"
	@echo ""


.INTERMEDIATE: $(SOURCEFILELIST) $(SUBT_SOURCEFILELIST)		\
		$(FULL_SOURCEFILELIST) $(ENV_SCRIPT_OUTPUT)

FORCE:

.PHONY: all FORCE clean clean-files clean-pkg clean-subtargets		\
	install install-bin install-bin-pkg install-dev install-dev-pkg	\
	post-install-script bin-pkg dev-pkg src-pkg backup-pkg pkg	\
	target subtargets debuginfo

clean-subtargets:
	@for t in $(_SUBTARGETS_DIRS) ; do				\
		$(SUBT_MAKE) -C $$t clean;				\
	done

clean-files: clean-subtargets
	@$(RM) -f $(BUILD_OUTPUT)*.d $(BUILD_OUTPUT)*.dd? $(BUILD_OUTPUT)*.o
	@for i in $(_EXTRA_DIRS) ; do				\
		$(RM) -f $(BUILD_OUTPUT)$${i}/*.d ; 		\
		$(RM) -f $(BUILD_OUTPUT)$${i}/*.dd? ;		\
		$(RM) -f $(BUILD_OUTPUT)$${i}/*.o ; 		\
	done
	@$(RM) -f "$(SOURCEFILELIST)" "$(SUBT_SOURCEFILELIST)"
	@$(RM) -f "$(FULL_SOURCEFILELIST)"
	@$(RM) -f $(TARGET) $(CTAGSTARGET) $(VISHEADER)
ifeq ($(TARGETTYPE),lib)
	@$(RM) -f "$(TARGET:.so=.a)"
endif
	@if [ -d "$(TMPDIR)" ]; then		\
		$(SUDORM) -rf "$(TMPDIR)" ;	\
	fi

clean-pkg:
	@$(RM) -f $(PKG) $(BINPKG) $(DEVPKG) $(SRCPKG) $(BAKPKG)

clean: clean-files clean-pkg
ifneq ($(BUILD_OUTPUT),)
	@for i in $$(find $(BUILD_OUTPUT) -mindepth 1 -type d | sort -r); do \
		$(RMDIR) $${i} ; done
	if [ -z "$$(ls -A $(BUILD_OUTPUT))" ]; then \
   		$(RMDIR) $(BUILD_OUTPUT); fi
endif
	@$(RM) -f "$(ENV_SCRIPT_OUTPUT)"

debuginfo: $(SOURCEFILELIST) $(SUBT_SOURCEFILELIST) $(FULL_SOURCEFILELIST)
	@echo "Release info:						"
	@echo " * Author:                  $(AUTHOR)			"
	@echo " * Commit:                  $(COMMIT)			"
	@echo " * Date:                    $(DATE)			"
	@echo " * Version:                 $(VERSION)			"
	@echo " * Version file:            $(VERSIONFILE)		"
	@echo "System info:						"
	@echo " * Shell:                   $(SHELL)			"
	@echo " * Enviroment script:       $(ENV_SCRIPT)		"
	@echo " * Env. script output file: $(ENV_SCRIPT_OUTPUT)		"
	@echo "Build configuration:					"
	@echo " * Alternate build:         $(BUILDALT)			"
	@echo " * Target name:             $(TARGETNAME)		"
	@echo " * Target type:             $(TARGETTYPE)		"
	@echo " * Target:                  $(TARGET) 	 		"
	@echo " * Debug flags:             $(DEBUG) => $(_DEBUG)	"
	@echo " * Optimize lib visibility: $(OPTIMIZE_LIB_VISIBILITY)	"
	@echo " * Visibility header:       $(VISHEADER)  		"
	@echo " * FS to build against:     $(BUILDFS)			"
	@echo " * Makefile list:					"
	@j=1; for i in $(MAKEFILE_LIST) ; do				\
				printf "   %3d. $$i\n" $$((j++)); done
	@echo "Compiler info:						"
	@echo " * Cross compile prefix:    $(CROSS_COMPILE)		"
	@echo " * CC:                      $(CC)			"
	@echo " * CXX:                     $(CXX)			"
	@echo " * CPP:                     $(CPP)			"
	@echo " * AR:                      $(AR)			"
	@echo " * GCC version:             $(CCVERSION)			"
	@echo " * User include flags:      $(INCFLAGS)			"
	@echo " * User extra CFLAGS:       $(EXTRA_CFLAGS)		"
	@echo " * CFLAGS:                  $(CFLAGS)			"
	@echo " * CXXFLAGS:                $(CXXFLAGS)			"
	@echo "Linker info:						"
	@echo " * Linker:                  $(LINK)			"
	@echo " * Static libs:             $(STATICLIBS)		"
	@echo " * Linker flags:            $(LDFLAGS)			"
	@echo "Sources:							"
	@echo " * Output build directory:  $(BUILD_OUTPUT)		"
	@echo " * Candidate extra dirs:					"
	@j=1; for i in $(EXTRA_DIRS) ; do printf "   %3d. $$i\n" $$((j++)); done
	@echo " * Extra dirs:						"
	@j=1; for i in $(_EXTRA_DIRS) ; do				\
				printf "   %3d. $$i\n" $$((j++)); done
	@echo " * Candidate subtargets:					"
	@j=1; for i in $(SUBTARGETS_DIRS) ; do				\
				printf "   %3d. $$i\n" $$((j++)); done
	@echo " * Subtargets:						"
	@j=1; for i in $(_SUBTARGETS_DIRS) ; do				\
				printf "   %3d. $$i\n" $$((j++)); done
	@echo " * Sources:						"
	@j=1; for i in $(SRC) ; do printf "   %3d. $$i\n" $$((j++)); done
	@echo " * Object files:						"
	@j=1; for i in $(OBJ) ; do printf "   %3d. $$i\n" $$((j++)); done
	@echo " * Dependency files (and their content):			"
	@j=1; for i in $(DEP) ; do					\
		printf "   %3d. $$i => $$(cat $$i)\n" $$((j++)); done
	@echo " * Source file list in:     $(SOURCEFILELIST) 		"
	@echo " * Content:						"
	@j=1; for i in $$(cat $(SOURCEFILELIST)) ; do			\
		printf "   %3d. $$i\n" $$((j++)); done
	@echo " * Subtargets source file list in: $(SUBT_SOURCEFILELIST)"
	@if test -s "$(SUBT_SOURCEFILELIST)" ; then			\
		echo " * Content:" ;					\
		j=1; for i in $$(cat $(SUBT_SOURCEFILELIST)) ; do	\
			printf "   %3d. $$i\n" $$((j++)); done ;	\
	else								\
		echo " * Content: file empty" ; fi
	@echo " * Full source file list in: $(FULL_SOURCEFILELIST) 	"
	@echo " * Content:						"
	@j=1; for i in $$(cat $(FULL_SOURCEFILELIST)) ; do		\
		printf "   %3d. $$i\n" $$((j++)); done
	@echo "Install info:						"
	@echo " * Use sudo:                $(USESUDO)			"
	@echo " * Install root:            $(INSTALL_ROOT)		"
	@echo " * Install prefix:          $(INSTALL_PREFIX)		"
	@echo " * Install directory:       $(INSTALL_DIR)		"
	@echo " * Libraries subdirectory:  $(LIBSUBDIR)			"
	@echo " * Headers install folder:  $(HEADERS_INSTALL_DIR)	"
	@echo " * Main header to install:  $(INSTALL_HEADER)		"
ifneq ($(MAIN_HEADER_DEP),)
	@echo " * Main header dependencies:				"
	@j=1; for i in $(_MAIN_HEADER_DEP) ;				\
		do printf "   %3d. $$i\n" $$((j++)); done
endif
	@echo " * Install target:          $(INSTALL_TARGET)  		"
	@echo " * Target headers:          $(TARGET_HEADERS)  		"
	@echo " * Build FS target:         $(BUILDFS_TARGET)  		"
	@echo " * Post install script:     $(POST_INSTALL_SCRIPT)	"
	@echo " * Post install command:    $(POST_INSTALL_SCRIPT_CMD)	"
	@echo "Packaging info:						"
	@echo " * Temp directory:          $(TMPDIR)			"
	@echo " * Package:                 $(PKG)			"
	@echo " * Binary package:          $(BINPKG)			"
	@echo " * Development package:     $(DEVPKG)			"
	@echo " * Source package folder:   $(SRCPKGDIR)			"
	@echo " * Source package:          $(SRCPKG)			"
	@echo " * Backup package folder:   $(BAKPKGDIR)			"
	@echo " * Backup package:          $(BAKPKG)			"
