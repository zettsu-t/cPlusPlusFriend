GCC_VERSION:=$(shell export LC_ALL=C ; gcc -v 2>&1 | tail -1 | cut -d " " -f 3)
GCC_VERSION_NUMBER:=$(shell export LC_ALL=C ; g++ -v 2>&1 | tail -1 | sed -e "s/.* \\([0-9]\\)\\.\\([0-9]\\).*/\\1.\\2/")

ifeq ($(OS),Windows_NT)
ifneq (,$(findstring cygwin,$(shell gcc -dumpmachine)))
BUILD_ON_CYGWIN=yes
HOME_MAKEFILE=$(shell cygpath $(HOME))

else
BUILD_ON_MINGW=yes
HOME_MAKEFILE=$(HOME)
ifeq (,$(findstring x86_64,$(shell gcc -dumpmachine)))
BUILD_ON_MINGW32=yes
CLANG_TARGET=-target i686-pc-windows-gnu
else
CLANG_TARGET=-target x86_64-pc-windows-gnu
endif

# We do not support a path with white spaces (C:\Program Files)
MINGW_DIR=C:\MinGW
ISYSTEM_MINGW_INCLUDE_DIR=$(MINGW_DIR)\include
ISYSTEM_MINGW_CLANG_INCLUDE_DIRS = $(ISYSTEM_MINGW_INCLUDE_DIR) $(ISYSTEM_MINGW_INCLUDE_DIR)\c++\$(GCC_VERSION) $(ISYSTEM_MINGW_INCLUDE_DIR)\c++\$(GCC_VERSION)\x86_64-w64-mingw32 $(MINGW_DIR)\x86_64-w64-mingw32\include

BOOST_LIB_POSTFIX=
endif
HOME_MAKEFILE=$(HOME)
endif

ifeq ($(BUILD_ON_MINGW),yes)
INCLUDES_GXX=$(addprefix -isystem ,$(ISYSTEM_MINGW_INCLUDE_DIR))
INCLUDES_CLANGXX=$(addprefix -isystem ,$(ISYSTEM_MINGW_CLANG_INCLUDE_DIRS))
endif

CLANGXX_COMMON_FLAGS=$(INCLUDES_CLANGXX)
ifeq ($(BUILD_ON_MINGW),yes)
CLANGXX_COMMON_FLAGS+=$(CLANG_TARGET) -fno-exceptions -DBOOST_EXCEPTION_DISABLE=0 -DBOOST_NO_EXCEPTIONS=0
CLANGXX_LDFLAGS=-Wl,--allow-multiple-definition
CLANGXX_LIBS=-lsupc++
else
CLANGXX_COMMON_FLAGS+=-D__STRICT_ANSI__
endif

CPPFLAGS=-std=c++2a -O2 -Wall -W -Wconversion -Wformat=2 -Wcast-qual -Wcast-align -Wwrite-strings -Wconversion -Wfloat-equal -Wpointer-arith -Wno-unused-parameter
CLANGXX_CFLAGS=$(CLANG_TARGET) $(INCLUDES_CLANGXX) $(CPPFLAGS)

SOURCE=warnings.cpp
OBJ_GXX=warnings_gcc.o
OBJ_CLANGXX=warnings_clang.o
OBJS=$(OBJ_GXX) $(OBJ_CLANGXX)
TARGET_GXX=warnings_gcc
TARGET_CLANGXX=warnings_clang
TARGETS=$(TARGET_GXX) $(TARGET_CLANGXX)

CXX=g++
CLANGXX=clang++
LD=g++
LIBPATH=
LDFLAGS=
LIBS=

ifneq ($(BUILD_ON_MINGW),yes)
ifneq ($(BUILD_ON_CYGWIN),yes)
# Workaround for Ubuntu 18.04 LTS on WSL2
CLANGXX_CFLAGS+=-fPIC
endif
endif

.PHONY: all run clean

all: $(TARGETS)

$(TARGET_GXX): $(OBJ_GXX)
	$(LD) $(LIBPATH) -o $@ $^ $(LDFLAGS) $(LIBS)
	./$@

$(TARGET_CLANGXX): $(OBJ_CLANGXX)
	$(LD) $(LIBPATH) -o $@ $^ $(LDFLAGS) $(LIBS) $(CLANGXX_LDFLAGS) $(CLANGXX_LIBS)
	./$@

$(OBJ_GXX): $(SOURCE) Makefile
	$(CXX) $(CPPFLAGS) -c -o $@ $<

$(OBJ_CLANGXX): $(SOURCE) Makefile
	$(CLANGXX) $(CLANGXX_CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGETS) $(OBJS)
