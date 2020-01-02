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

SOURCE=cpp_optimize.cpp
ASM_GCC=cpp_optimize_gcc.s
ASM_CLANG=cpp_optimize_clang.s
OBJ_CLANG=cpp_optimize_clang.o
TARGET_GCC=cpp_optimize_gcc
TARGET_CLANG=cpp_optimize_clang

TARGETS=$(TARGET_GCC) $(TARGET_CLANG)
OBJS=$(OBJ_CLANG)
ASMS=$(ASM_GCC) $(ASM_CLANG)

CXXFLAGS_COMMON=-std=c++17 -O3 -mavx2 -Wall -W -Wconversion -Wformat=2 -Wcast-qual -Wcast-align -Wwrite-strings -Wconversion -Wfloat-equal -Wpointer-arith -Wno-unused-parameter
CASMFLAGS=-S -masm=intel
GXX_CFLAGS=$(INCLUDES_GXX) $(CXXFLAGS_COMMON)
CLANGXX_CFLAGS=$(CLANG_TARGET) $(INCLUDES_CLANGXX) $(CXXFLAGS_COMMON)

.PHONY: all clean rebuild

all: $(TARGETS)

$(TARGET_GCC) : $(SOURCE)
	g++ -v
	g++ $(GXX_CFLAGS) $(CASMFLAGS) -o $(ASM_GCC) $<
	g++ $(GXX_CFLAGS) -o $@ $<
	time ./$@

$(TARGET_CLANG) : $(SOURCE)
	clang++ -v
	clang++ $(CLANGXX_CFLAGS) $(CASMFLAGS) -o $(ASM_CLANG) $<
	clang++ $(CLANGXX_CFLAGS) -c -o $(OBJ_CLANG) $<
	g++ -o $@ $(OBJ_CLANG)
	time ./$@

clean:
	$(RM) $(TARGETS) $(OBJS) $(ASMS)

rebuild : clean all
