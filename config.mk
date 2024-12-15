# Copyright (c) 2024
# Author: Andrea Pepe <pepe.andmj@gmail.com>
#
# Version 0.1.1 (Dec 14th, 2024)


# Please set your own target name.
# Note that when building libraries the final library name will be:
# - lib$(TARGETNAME).so for shared libraries
# - lib$(TARGETNAME).a for static libraries
TARGETNAME=zoro

# Target type can be:
# - 'exec' for dynamic executable
# - 'staticexec' for static executable
# - 'lib' to build as library (both shared and static)
# - 'sharedlib' for shared library
# - 'staticlib' for static library
TARGETTYPE=lib

# Define debug features (multiple flags are allowed)
#  n -> all debug features disabled
#  y -> basic debug flags
#  a -> ASAN support
#  t -> TSAN support -- ignored if 'a' is given
#  s -> GCC static analyzer (since GCC v10)
#  Y -> same as 'ys'
#  A -> same as 'ysa'
#  T -> same as 'yst'
# DEBUG=n

# 'y' -> if you want this Makefile to use 'sudo' when installing target or
# 	 creating a package
# 'n' -> if you want to type 'sudo' yourself whenever you think you need to.
# USESUDO=n

# 'y' -> if you want to run GCC inside GDB to debug compilation process
# 'n' -> otherwise (std. behaviour)
# USEGDB=n

# Change the build output directory (default is .)
# BUILD_OUTPUT=.

# Add here extra include directories (EXTRA_DIRS are automatically added)
INCFLAGS=-Iinclude

# Add here your -L linker options
# LDFLAGS=

# Add here your -l linker options
# Remember to add -lstdc++ when linking C++ code
# LDLIBS=

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
#
# STATICLIBS=

# file name for version file
# VERSIONFILE=VERSION

# Author name
AUTHOR=Andrea Pepe

# Default verbose configuration
# 0 -> pretty output
# 1 -> verbose output (build and clean commands shown)
# 2 -> very verbose output (all commands are shown)
# DEFAULT_VERBOSE=0

# When building libraries set this variable to 'y' to manually select which
# function will be available through the library. If you choose to do this,
# remember to mark "__public" all the functions you want to export.
# For example:
#                 int __public mypublicfunc(void) { ... }
#
# OPTIMIZE_LIB_VISIBILITY=n

# Use this feature if you need to load a script that changes the environment
# for all the commands executed in this makefile
# i.e.: to me it is useful when I'm cross compiling to add the cross tools
#       to the PATH and LD_LIBRARY_PATH.
# ENV_SCRIPT=

# Build also sources from all the directories listed in EXTRA_DIRS
# By default, files from all subdirectories are built.
# If you want to build only few directories, just list them in the variable.
# Not to include any directory, just leave the variable empty
# EXTRA_DIRS=
# SUBTARGETS_DIRS=
ifeq ($(TEST),)
SUBTARGETS_DIRS=
endif

# Uncomment (and eventually change the header file name)
# to install an header file along with your target
# (this is very common for libreries)
# It is allowed only one single file with extension .h
# Any file that it includes using doblue quotes (not angle brackets!) is going
# to be installed as well
INSTALL_HEADER=include/$(TARGETNAME).h

# The filesystem which you build against
# Note that in this file system will also be installed header files and libraries.
# BUILDFS=/

# The filesystem where you want your binary files (executables and shared
# libraries) to be installed into
# INSTALL_ROOT=$(BUILDFS)

# INSTALL_PREFIX=/usr/local

# INSTALL_PREFIX's subdirectory where to install targets
# Leave it commented to use defaults:
# - 'lib' (or lib64) for libraries, 'bin' for executables
# INSTALL_DIR=

# Cross compiler prefix, e.g. like "arm-arago-linux-gnueabi-"
# CROSS_COMPILE=
