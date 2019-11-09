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

HEADERS=cmp_bool.hpp
SOURCE_MAIN=cmp_bool_main.cpp
SOURCE_EXT=cmp_bool_ext.cpp

OBJ_MAIN_GCC=cmp_bool_main_gcc.o
OBJ_EXT_GCC=cmp_bool_ext_gcc.o
OBJ_MAIN_CLANG=cmp_bool_main_clang.o
OBJ_EXT_CLANG=cmp_bool_ext_clang.o
OBJ_EXT_CLANG_ALT=cmp_bool_ext_clang_alt.o
OBJ_EXT_GCC_NO_OPT=cmp_bool_ext_gcc_no_opt.o
OBJ_EXT_CLANG_NO_OPT=cmp_bool_ext_clang_no_opt.o

OBJS_GCC=$(OBJ_MAIN_GCC) $(OBJ_EXT_GCC)
OBJS_CLANG=$(OBJ_MAIN_CLANG) $(OBJ_EXT_CLANG)
OBJS_CLANG_ALT=$(OBJ_MAIN_CLANG) $(OBJ_EXT_CLANG_ALT)
OBJS=$(OBJS_GCC) $(OBJS_CLANG) $(OBJS_CLANG_ALT) $(OBJ_EXT_GCC_NO_OPT) $(OBJ_EXT_CLANG_NO_OPT)

TARGET_GCC=cmp_bool_gcc
TARGET_CLANG=cmp_bool_clang
TARGET_CLANG_ALT=cmp_bool_clang_alt

DISASM_EXT_GCC=cmp_bool_ext_gcc_disasm.txt
DISASM_EXT_CLANG=cmp_bool_ext_clang_disasm.txt
DISASM_EXT_CLANG_ALT=cmp_bool_ext_clang_alt_disasm.txt
DISASM_EXT_GCC_NO_OPT=cmp_bool_ext_gcc_disasm_no_opt.txt
DISASM_EXT_CLANG_NO_OPT=cmp_bool_ext_clang_disasm_no_opt.txt

DISASM_FILES=$(DISASM_EXT_GCC) $(DISASM_EXT_CLANG) $(DISASM_EXT_CLANG_ALT) $(DISASM_EXT_GCC_NO_OPT) $(DISASM_EXT_CLANG_NO_OPT)
TARGETS=$(DISASM_FILES) $(TARGET_GCC) $(TARGET_CLANG) $(TARGET_CLANG_ALT)

GXX=g++
CLANGXX=clang++
LD=g++
OPT_FULL=-O3
OPT_DISABLED=-O0
DISASM=objdump -d -M intel

CPPFLAGS=-std=gnu++17 -Wall
GXX_CPPFLAGS=$(CPPFLAGS) $(INCLUDES_GXX)
CLANGXX_CPPFLAGS=$(CPPFLAGS) $(INCLUDES_CLANGXX) $(CLANG_TARGET)
CLANGXX_CPPFLAGS_ALT=$(CLANGXX_CPPFLAGS) -fstrict-enums
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

$(TARGET_CLANG_ALT): $(OBJS_CLANG_ALT)
	$(LD) $(LIBPATH) -o $@ $^ $(LDFLAGS) $(LIBS)

run: $(TARGETS)
	@echo -e $(ECHO_START)$(TARGET_GCC)$(ECHO_END)
	-./$(TARGET_GCC)
	@echo -e $(ECHO_START)$(TARGET_CLANG)$(ECHO_END)
	-./$(TARGET_CLANG)
	@echo -e $(ECHO_START)$(TARGET_CLANG_ALT)$(ECHO_END)
	-./$(TARGET_CLANG_ALT)

$(OBJ_MAIN_GCC): $(SOURCE_MAIN) $(HEADERS)
	$(GXX) $(OPT_FULL) $(GXX_CPPFLAGS) -o $@ -c $<

$(OBJ_MAIN_CLANG): $(SOURCE_MAIN) $(HEADERS)
	$(CLANGXX) $(OPT_FULL) $(CLANGXX_CPPFLAGS) -o $@ -c $<

$(OBJ_EXT_GCC): $(SOURCE_EXT) $(HEADERS)
	$(GXX) $(OPT_FULL) $(GXX_CPPFLAGS) -o $@ -c $<

$(DISASM_EXT_GCC): $(OBJ_EXT_GCC)
	$(DISASM) $< > $@

$(OBJ_EXT_CLANG): $(SOURCE_EXT) $(HEADERS)
	$(CLANGXX) $(OPT_FULL) $(CLANGXX_CPPFLAGS) -o $@ -c $<

$(DISASM_EXT_CLANG): $(OBJ_EXT_CLANG)
	$(DISASM) $< > $@

$(OBJ_EXT_CLANG_ALT): $(SOURCE_EXT) $(HEADERS)
	$(CLANGXX) $(OPT_FULL) $(CLANGXX_CPPFLAGS_ALT) -o $@ -c $<

$(DISASM_EXT_CLANG_ALT): $(OBJ_EXT_CLANG_ALT)
	$(DISASM) $< > $@

$(OBJ_EXT_GCC_NO_OPT): $(SOURCE_EXT) $(HEADERS)
	$(GXX) $(OPT_DISABLED) $(GXX_CPPFLAGS) -o $@ -c $<

$(DISASM_EXT_GCC_NO_OPT): $(OBJ_EXT_GCC_NO_OPT)
	$(DISASM) $< > $@

$(OBJ_EXT_CLANG_NO_OPT): $(SOURCE_EXT) $(HEADERS)
	$(CLANGXX) $(OPT_DISABLED) $(CLANGXX_CPPFLAGS) -o $@ -c $<

$(DISASM_EXT_CLANG_NO_OPT): $(OBJ_EXT_CLANG_NO_OPT)
	$(DISASM) $< > $@

clean:
	rm -f $(TARGETS) $(OBJS) $(DISASM_FILES)
