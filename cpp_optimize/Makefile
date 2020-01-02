GCC_VERSION:=$(shell export LC_ALL=C ; gcc -v 2>&1 | tail -1 | cut -d " " -f 3)
GCC_VERSION_NUMBER:=$(shell export LC_ALL=C ; g++ -v 2>&1 | tail -1 | sed -e "s/.* \\([0-9]\\)\\.\\([0-9]\\).*/\\1.\\2/")
LLVM_VERSION:=$(shell export LC_ALL=C ; clang++ -v 2>&1 | head -1 | sed -e "s/.* \\([0-9]\\)\\.\\([0-9]\\)\\b.*/\\1\\2/" | cut -c 1-2)

ifeq ($(OS),Windows_NT)
ifneq (,$(findstring cygwin,$(shell gcc -dumpmachine)))
BUILD_ON_CYGWIN=yes

else
BUILD_ON_MINGW=yes
ifeq (,$(findstring x86_64,$(shell gcc -dumpmachine)))
BUILD_ON_MINGW32=yes
CLANG_TARGET=-target i686-pc-windows-gnu
else
CLANG_TARGET=-target x86_64-pc-windows-gnu
endif

MINGW_DIR=C:\MinGW
ISYSTEM_MINGW_INCLUDE_DIR=$(MINGW_DIR)\include
ISYSTEM_MINGW_CLANG_INCLUDE_DIRS = $(ISYSTEM_MINGW_INCLUDE_DIR) $(ISYSTEM_MINGW_INCLUDE_DIR)\c++\$(GCC_VERSION) $(ISYSTEM_MINGW_INCLUDE_DIR)\c++\$(GCC_VERSION)\x86_64-w64-mingw32 $(MINGW_DIR)\x86_64-w64-mingw32\include
endif
endif

ifeq ($(BUILD_ON_MINGW),yes)
INCLUDES_GXX=$(addprefix -isystem ,$(ISYSTEM_MINGW_INCLUDE_DIR))
INCLUDES_CLANGXX=$(addprefix -isystem ,$(ISYSTEM_MINGW_CLANG_INCLUDE_DIRS))
endif

## double precision SIMD oriented Fast Mersenne Twister (dSFMT)
## Set a directory in which dSFMT.h exists to DSFMT_PATH
DSFMT_PATH=$(HOME)/dSFMT
INCLUDES_DSFMT=$(addprefix -isystem ,$(DSFMT_PATH))

DSFMT_SOURCE=dSFMT.c
OBJ_DSMFT_GCC=dsfmt_gcc.o
OBJ_DSMFT_CLANG=dsfmt_clang.o

SOURCE=cpp_optimize.cpp
ASM_GCC=cpp_optimize_gcc.s
ASM_CLANG=cpp_optimize_clang.s
ASMS=$(ASM_GCC) $(ASM_CLANG)

OBJ_GCC=cpp_optimize_gcc.o
OBJ_CLANG=cpp_optimize_clang.o
OBJS_GCC=$(OBJ_GCC) $(OBJ_DSMFT_GCC)
OBJS_CLANG=$(OBJ_CLANG) $(OBJ_DSMFT_CLANG)
OBJS=$(OBJS_GCC) $(OBJS_CLANG)

TARGET_GCC=cpp_optimize_gcc
TARGET_CLANG=cpp_optimize_clang
TARGETS=$(TARGET_GCC) $(TARGET_CLANG)

OPT=-O3
STD=-std=c++17
CFLAGS_DSFMT=$(INCLUDES_DSFMT) -DDSFMT_MEXP=19937
CFLAGS_COMMON=$(OPT) $(CFLAGS_DSFMT) -mavx2 -Wall -W -Wconversion -Wformat=2 -Wcast-qual -Wcast-align -Wwrite-strings -Wconversion -Wfloat-equal -Wpointer-arith -Wno-unused-parameter
CASMFLAGS=-S -masm=intel
GXX_CFLAGS=$(INCLUDES_GXX) $(CFLAGS_COMMON)
CLANGXX_CFLAGS=$(CLANG_TARGET) $(INCLUDES_CLANGXX) $(CFLAGS_COMMON)
GXX_CPPFLAGS=$(STD) $(GXX_CFLAGS)
CLANGXX_CPPFLAGS=$(STD) $(CLANGXX_CFLAGS)

.PHONY: all run clean rebuild

ARGS_TARGETS=100000000 10
VPATH=$(DSFMT_PATH)

all: $(TARGETS)

$(TARGET_GCC) : $(OBJS_GCC)
	g++ -o $@ $^

$(TARGET_CLANG) : $(OBJS_CLANG)
	g++ -o $@ $^

run: $(TARGETS)
	./$(TARGET_GCC)
	./$(TARGET_CLANG)
	./$(TARGET_GCC) $(ARGS_TARGETS)
	./$(TARGET_CLANG) $(ARGS_TARGETS)

$(OBJ_GCC) : $(SOURCE) Makefile
	g++ -v
	g++ $(GXX_CPPFLAGS) $(CASMFLAGS) -o $(ASM_GCC) $<
	g++ $(GXX_CPPFLAGS) -c -o $@ $<

$(OBJ_CLANG) : $(SOURCE) Makefile
	clang++ -v
	clang++ $(CLANGXX_CPPFLAGS) $(CASMFLAGS) -o $(ASM_CLANG) $<
	clang++ $(CLANGXX_CPPFLAGS) -c -o $@ $<

$(OBJ_DSMFT_GCC) : $(DSFMT_SOURCE) Makefile
	gcc $(GXX_CFLAGS) -c -o $@ $<

$(OBJ_DSMFT_CLANG) : $(DSFMT_SOURCE) Makefile
	clang $(CLANGXX_CFLAGS) -c -o $@ $<

clean:
	$(RM) $(TARGETS) $(OBJS) $(ASMS)

rebuild : clean all
