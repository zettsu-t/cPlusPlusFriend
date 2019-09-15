SOURCE=template_params.cpp
OBJ_CLANGXX_CPP17=template_params_cpp17.o
OBJ_CLANGXX_CPP14=template_params_cpp14.o
OBJS=$(OBJ_CLANGXX_CPP17) $(OBJ_CLANGXX_CPP14)
TARGET_GXX=template_params_gcc
TARGET_CLANGXX_CPP17=template_params_clang_cpp17
TARGET_CLANGXX_CPP14=template_params_clang_cpp14
TARGETS=$(TARGET_GXX) $(TARGET_CLANGXX_CPP17) $(TARGET_CLANGXX_CPP14)

LD=g++
GXX=g++
CLANGXX=clang++
CPPFLAGS=-g -O0 -Wall
LIBPATH=
LDFLAGS=
LIBS=

GCC_VERSION:=$(shell export LC_ALL=C ; gcc -v 2>&1 | tail -1 | cut -d " " -f 3)
GCC_VERSION_NUMBER:=$(shell export LC_ALL=C ; g++ -v 2>&1 | tail -1 | sed -e "s/.* \\([0-9]\\)\\.\\([0-9]\\).*/\\1.\\2/")
LLVM_VERSION:=$(shell export LC_ALL=C ; clang++ -v 2>&1 | head -1 | sed -e "s/.* \\([0-9]\\)\\.\\([0-9]\\)\\b.*/\\1\\2/" | cut -c 1-2)

ifeq ($(OS),Windows_NT)
ifneq (,$(findstring cygwin,$(shell gcc -dumpmachine)))
BUILD_ON_CYGWIN=yes
else
BUILD_ON_MINGW=yes
CLANG_TARGET=-target x86_64-pc-windows-gnu
endif
endif

MINGW_DIR=C:\MinGW
ISYSTEM_MINGW_INCLUDE_DIR=$(MINGW_DIR)\include
ISYSTEM_MINGW_CLANG_INCLUDE_DIRS = $(ISYSTEM_MINGW_INCLUDE_DIR) $(ISYSTEM_MINGW_INCLUDE_DIR)\c++\$(GCC_VERSION) $(ISYSTEM_MINGW_INCLUDE_DIR)\c++\$(GCC_VERSION)\x86_64-w64-mingw32 $(MINGW_DIR)\x86_64-w64-mingw32\include

ifeq ($(BUILD_ON_MINGW),yes)
INCLUDES_GXX=$(addprefix -isystem ,$(ISYSTEM_MINGW_INCLUDE_DIR))
INCLUDES_CLANGXX=$(addprefix -isystem ,$(ISYSTEM_MINGW_CLANG_INCLUDE_DIRS))
endif

GXX_CFLAGS=$(INCLUDES_GXX) $(CPPFLAGS)
CLANGXX_CFLAGS=$(CLANG_TARGET) $(INCLUDES_CLANGXX) $(CPPFLAGS) -frelaxed-template-template-args

.PHONY: all clean

all: $(TARGETS)

$(TARGET_GXX): $(SOURCE)
	$(GXX) -std=gnu++17 $(GXX_CFLAGS) $(LIBPATH) -o $@ $< $(LDFLAGS) $(LIBS)
	./$@
	-$(GXX) -std=gnu++14 $(GXX_CFLAGS) $(LIBPATH) -o $@ $< $(LDFLAGS) $(LIBS)

$(TARGET_CLANGXX_CPP17): $(OBJ_CLANGXX_CPP17)
	$(LD) $(LIBPATH) -o $@ $^ $(LDFLAGS) $(LIBS)
	./$@

$(TARGET_CLANGXX_CPP14): $(OBJ_CLANGXX_CPP14)
	$(LD) $(LIBPATH) -o $@ $^ $(LDFLAGS) $(LIBS)
	./$@

$(OBJ_CLANGXX_CPP17): $(SOURCE)
	$(CLANGXX) -std=gnu++17 $(CLANGXX_CFLAGS) -o $@ -c $<

$(OBJ_CLANGXX_CPP14): $(SOURCE)
	-$(CLANGXX) -std=gnu++14 $(CLANGXX_CFLAGS) -o $@ -c $<

clean:
	rm -f $(TARGETS) $(OBJS)
