GTEST_GMOCK_TOP_DIR=$(HOME)/googletest
GTEST_TOP_DIR=$(GTEST_GMOCK_TOP_DIR)/googletest
GTEST_GMOCK_INCLUDE=$(addprefix -isystem, $(GTEST_TOP_DIR)/include $(GTEST_TOP_DIR) $(GMOCK_TOP_DIR)/include $(GMOCK_TOP_DIR))
GTEST_SOURCE=$(GTEST_TOP_DIR)/src/gtest-all.cc
GTEST_OBJ_GCC=$(patsubst %.cc, %_gcc.o, $(notdir $(GTEST_SOURCE)))
GTEST_OBJ_CLANG=$(patsubst %.cc, %_clang.o, $(notdir $(GTEST_SOURCE)))

SOURCE_CPP=popcount_cpp.cpp
SOURCE_ASM=popcount_asm.s

OBJ_CPP_GCC=popcount_cpp_gcc.o
OBJ_CPP_CLANG=popcount_cpp_clang.o
OBJ_ASM=popcount_asm.o
OBJS_GCC=$(OBJ_CPP_GCC) $(OBJ_ASM) $(GTEST_OBJ_GCC)
OBJS_CLANG=$(OBJ_CPP_CLANG) $(OBJ_ASM) $(GTEST_OBJ_CLANG)

TARGET_GCC=popcount_gcc
TARGET_CLANG=popcount_clang
TARGETS=$(TARGET_GCC) $(TARGET_CLANG)

AS=as
CXX=g++
CLANGXX=clang++
OPT=-O3

LD=g++
CPPFLAGS=-std=gnu++17 $(OPT) -mavx2 -Wall -W -Wconversion -Wformat=2 -Wcast-qual -Wcast-align -Wwrite-strings -Wconversion -Wfloat-equal -Wpointer-arith -Wno-unused-parameter $(GTEST_GMOCK_INCLUDE)

LIBPATH=
LDFLAGS=
LIBS=

ifeq (,$(findstring cygwin,$(shell gcc -dumpmachine)))
CPPFLAGS+=-isystem C:\MinGW\include
ifeq (clang++,$(CXX))
## CLANG_TARGET=-target x86_64-pc-windows-gnu
endif
endif

.PHONY: all run clean

all: $(TARGETS)

$(TARGET_GCC): $(OBJS_GCC)
	$(LD) $(LIBPATH) -o $@ $^ $(LDFLAGS) $(LIBS)

$(TARGET_CLANG): $(OBJS_CLANG)
	$(LD) $(LIBPATH) -o $@ $^ $(LDFLAGS) $(LIBS)

run : $(TARGETS)
	./$(TARGET_GCC)
	./$(TARGET_CLANG)

$(OBJ_CPP_GCC): $(SOURCE_CPP)
	$(CXX) $(CPPFLAGS) -o $@ -c $<

$(OBJ_CPP_CLANG): $(SOURCE_CPP)
	$(CLANGXX) $(CLANG_TARGET) $(CPPFLAGS) -o $@ -c $<

$(OBJ_ASM): $(SOURCE_ASM)
	$(AS) -o $@ $<

$(GTEST_OBJ_GCC): $(GTEST_SOURCE)
	$(CXX) $(CPPFLAGS) -o $@ -c $<

$(GTEST_OBJ_CLANG): $(GTEST_SOURCE)
	$(CLANGXX) $(CLANG_TARGET) $(CPPFLAGS) -o $@ -c $<

clean:
	rm -f $(TARGETS) $(OBJS_GCC) $(OBJS_CLANG)
