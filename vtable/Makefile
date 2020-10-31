GCC_VERSION:=$(shell export LC_ALL=C ; gcc -v 2>&1 | tail -1 | cut -d " " -f 3)
GCC_VERSION_NUMBER:=$(shell export LC_ALL=C ; g++ -v 2>&1 | tail -1 | sed -e "s/.* \\([0-9]\\)\\.\\([0-9]\\).*/\\1.\\2/")

MINGW_DIR=C:\MinGW
ISYSTEM_MINGW_INCLUDE_DIR=$(MINGW_DIR)\include
ISYSTEM_MINGW_CLANG_INCLUDE_DIRS = $(ISYSTEM_MINGW_INCLUDE_DIR) $(ISYSTEM_MINGW_INCLUDE_DIR)\c++\$(GCC_VERSION) $(ISYSTEM_MINGW_INCLUDE_DIR)\c++\$(GCC_VERSION)\x86_64-w64-mingw32 $(MINGW_DIR)\x86_64-w64-mingw32\include

INCLUDES_GXX=
INCLUDES_CLANGXX=
CLANG_TARGET=

ifeq ($(OS),Windows_NT)
ifneq (,$(findstring cygwin,$(shell gcc -dumpmachine)))
BUILD_ON_CYGWIN=yes
else
BUILD_ON_MINGW=yes
INCLUDES_GXX=$(addprefix -isystem ,$(ISYSTEM_MINGW_INCLUDE_DIR))
INCLUDES_CLANGXX=$(addprefix -isystem ,$(ISYSTEM_MINGW_CLANG_INCLUDE_DIRS))
CLANG_TARGET=-target x86_64-pc-windows-gnu
endif
endif

HEADERS=vtable.hpp
SOURCE_MAIN=vtable.cpp
SOURCE_DEF=vtable_def.cpp
OBJ_MAIN_GCC=vtable_gcc.o
OBJ_DEF_GCC=vtable_def_gcc.o
OBJ_MAIN_CLANG=vtable_clang.o
OBJ_DEF_CLANG=vtable_def_clang.o

OBJS_GCC=$(OBJ_MAIN_GCC) $(OBJ_DEF_GCC)
OBJS_CLANG=$(OBJ_MAIN_CLANG) $(OBJ_DEF_CLANG)
OBJS=$(OBJS_GCC) $(OBJS_CLANG)

TARGET_GCC=vtable_gcc
TARGET_CLANG=vtable_clang
TARGETS=$(TARGET_GCC) $(TARGET_CLANG)

GXX=g++
CLANGXX=clang++
LD=g++
OPT=-O2

CPPFLAGS=-std=gnu++17 -Wall $(OPT)
GXX_CPPFLAGS=$(CPPFLAGS) $(INCLUDES_GXX)
CLANGXX_CPPFLAGS=$(CPPFLAGS) $(INCLUDES_CLANGXX) $(CLANG_TARGET)
LIBPATH=
LDFLAGS=
LIBS=

ECHO_START="\e[104m
ECHO_END=\e[0m"

.PHONY: all run clean

all: $(TARGETS)

$(TARGET_GCC): $(OBJS_GCC)
	$(LD) $(LIBPATH) -o $@ $^ $(LDFLAGS) $(LIBS)

$(TARGET_CLANG): $(OBJS_CLANG)
	$(LD) $(LIBPATH) -o $@ $^ $(LDFLAGS) $(LIBS)

run: $(TARGETS)
	@echo -e $(ECHO_START)$(TARGET_GCC)$(ECHO_END)
	-./$(TARGET_GCC)
	@echo -e $(ECHO_START)$(TARGET_CLANG)$(ECHO_END)
	-./$(TARGET_CLANG)

$(OBJ_MAIN_GCC): $(SOURCE_MAIN) $(HEADERS)
	$(GXX) $(GXX_CPPFLAGS) -o $@ -c $<

$(OBJ_DEF_GCC): $(SOURCE_DEF) $(HEADERS)
	$(GXX) $(GXX_CPPFLAGS) -o $@ -c $<

$(OBJ_MAIN_CLANG): $(SOURCE_MAIN) $(HEADERS)
	$(CLANGXX) $(CLANGXX_CPPFLAGS) -o $@ -c $<

$(OBJ_DEF_CLANG): $(SOURCE_DEF) $(HEADERS)
	$(CLANGXX) $(CLANGXX_CPPFLAGS) -o $@ -c $<

clean:
	rm -f $(TARGETS) $(OBJS)
