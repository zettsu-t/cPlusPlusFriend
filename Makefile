# ビルド環境を識別する
GCC_VERSION:=$(shell export LC_ALL=C ; gcc -v 2>&1 | tail -1 | cut -d " " -f 3)
GCC_VERSION_NUMBER:=$(shell export LC_ALL=C ; g++ -v 2>&1 | tail -1 | sed -e "s/.* \\([0-9]\\)\\.\\([0-9]\\).*/\\1.\\2/")
LLVM_VERSION:=$(shell export LC_ALL=C ; clang++ -v 2>&1 | head -1 | sed -e "s/.* \\([0-9]\\)\\.\\([0-9]\\)\\b.*/\\1\\2/" | cut -c 1-2)

# Structured bindingを使う
GCC_CPP17_VERSION=7.0
GCC_CPP17_AVAILABLE=$(shell echo "$(GCC_VERSION_NUMBER) >= $(GCC_CPP17_VERSION)" | bc)

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

# MinGWからstripコマンドを使うときは、明示的な拡張子が必要
EXPLICIT_EXE_SUFFIX=.exe
#パスにスペースが入っている(C:\Program Files)状況には対応しない
MINGW_DIR=C:\MinGW
# MinGW-32向けには書き換える必要がある
ISYSTEM_MINGW_INCLUDE_DIR=$(MINGW_DIR)\include
ISYSTEM_MINGW_CLANG_INCLUDE_DIRS = $(ISYSTEM_MINGW_INCLUDE_DIR) $(ISYSTEM_MINGW_INCLUDE_DIR)\c++\$(GCC_VERSION) $(ISYSTEM_MINGW_INCLUDE_DIR)\c++\$(GCC_VERSION)\x86_64-w64-mingw32 $(MINGW_DIR)\x86_64-w64-mingw32\include

# 必要であれば設定する
BOOST_LIB_POSTFIX=
endif

HOME_MAKEFILE=$(HOME)
endif

# コンソール出力に色を付ける
ECHO_START="\e[104m
ECHO_END=\e[0m"

# Google Test / Google Mockがあるディレクトリ
GTEST_GMOCK_TOP_DIR=$(HOME_MAKEFILE)/googletest
GTEST_TOP_DIR=$(GTEST_GMOCK_TOP_DIR)/googletest
GMOCK_TOP_DIR=$(GTEST_GMOCK_TOP_DIR)/googlemock
GTEST_GMOCK_INCLUDE=$(addprefix -isystem, $(GTEST_TOP_DIR)/include $(GTEST_TOP_DIR) $(GMOCK_TOP_DIR)/include $(GMOCK_TOP_DIR))
GTEST_SOURCE=$(GTEST_TOP_DIR)/src/gtest-all.cc
GMOCK_SOURCE=$(GMOCK_TOP_DIR)/src/gmock-all.cc

GTEST_OBJ=$(patsubst %.cc, %.o, $(notdir $(GTEST_SOURCE)))
GTEST_OBJ_LTO=$(patsubst %.cc, %_lto.o, $(notdir $(GTEST_SOURCE)))
GTEST_BC=$(patsubst %.cc, %.bc, $(notdir $(GTEST_SOURCE)))
GTEST_BC_LTO=$(patsubst %.cc, %_lto.bc, $(notdir $(GTEST_SOURCE)))

PREPROCESSED_CLANG_TEST=cppFriendsClangTest.processed

TARGET=cppFriends
TARGET_NO_OPT=cppFriends_no_opt
TARGET_C_SJIS=cFriendsShiftJis
TARGET_C=cFriends
# Clang for Windows にはllvm-linkがない
ifneq ($(BUILD_ON_MINGW),yes)
TARGET_GCC_LTO=cppFriends_gcc_lto
TARGET_CLANG=cppFriendsClang
TARGET_CLANG_LTO=cppFriendsClang_lto
endif
TARGET_RCPP=cppFriendsRcpp
OUTPUT_ASM87_C=cFriends87.s
OUTPUT_ASM87_STORE_C=cFriends87-store.s
OUTPUT_ASM64_C=cFriends64.s
OUTPUT_ASM64_CLANG=cFriends64_clang.s
OUTPUT_ASM_C_EXT1=cFriendsNoSideEffect.s
OUTPUT_ASM_C_EXT2=cFriendsSideEffect.s
OUTPUT_ASM_CPP_CLANG=cppFriendsClang_clang.s
OUTPUT_ASM_CPP_GCC=cppFriendsClang_gcc.s
OUTPUT_ASM_SINGLETON_11=cppFriendsSingleton.s
# C++11 or newer is required for Google Test
# OUTPUT_ASM_SINGLETON_DEFAULT=cppFriendsSingleton_thread_safe.s
# OUTPUT_ASM_SINGLETON_NO=cppFriendsSingleton_no_thread_safe.s

OUTPUT_ASMS=$(OUTPUT_ASM87_C) $(OUTPUT_ASM87_STORE_C) $(OUTPUT_ASM64_C) $(OUTPUT_ASM64_CLANG) $(OUTPUT_ASM_C_EXT1) $(OUTPUT_ASM_C_EXT2) $(OUTPUT_ASM_CPP_CLANG) $(OUTPUT_ASM_CPP_GCC) $(OUTPUT_ASM_SINGLETON_11) $(OUTPUT_ASM_SINGLETON_DEFAULT) $(OUTPUT_ASM_SINGLETON_NO)

OUTPUT_GENERATED_HEADER=cppFriendsAutoGenerated.hpp
OUTPUT_SAMPLE_2_DEPEND_GCC=cppFriendsSample2_gcc.d
OUTPUT_SAMPLE_2_DEPEND_GCC_I=cppFriendsSample2_gcc_i.d
OUTPUT_SAMPLE_2_DEPEND_CLANG=cppFriendsSample2_clang.d
OUTPUT_SAMPLE_2_DEPEND_CLANG_I=cppFriendsSample2_clang_i.d
OUTPUT_DEPENDS = $(OUTPUT_SAMPLE_2_DEPEND_GCC) $(OUTPUT_SAMPLE_2_DEPEND_GCC_I)
# Windows版clang++は、-MMでシステムヘッダを見つけられない
ifneq ($(BUILD_ON_MINGW),yes)
OUTPUT_DEPENDS += $(OUTPUT_SAMPLE_2_DEPEND_CLANG) $(OUTPUT_SAMPLE_2_DEPEND_CLANG_I)
endif
OUTPUT_LOG_STDOUT=__stdout_log.txt
OUTPUT_LOG_STDERR=__stderr_log.txt
OUTPUT_LOGS=$(OUTPUT_LOG_STDOUT) $(OUTPUT_LOG_STDERR)

OUTPUT_FUNCLIST_TEMP1_H=__cFriends_nocr1__.h
OUTPUT_FUNCLIST_TEMP1_C=__cFriends_nocr1__.c
OUTPUT_FUNCLIST_TEMP2_H=__cFriends_nocr2__.h
OUTPUT_FUNCLIST_TEMP2_C=__cFriends_nocr2__.c
OUTPUT_FUNCLIST_TEMP_SRCS=$(OUTPUT_FUNCLIST_TEMP2_H) $(OUTPUT_FUNCLIST_TEMP2_C)
OUTPUT_FUNCLIST_TEMPS=$(OUTPUT_FUNCLIST_TEMP1_H) $(OUTPUT_FUNCLIST_TEMP1_C) $(OUTPUT_FUNCLIST_TEMP_SRCS)
OUTPUT_FUNCLIST=cFriends_func.txt

OUTPUT_SAMPLE_2_HEADERS=__cppFriendsSample2.headers
OUTPUT_PREPROCESSED_FILES=$(OUTPUT_SAMPLE_2_HEADERS)

TARGETS=$(PREPROCESSED_CLANG_TEST) $(TARGET) $(OUTPUT_DEPENDS)
TARGETS+=$(TARGET_NO_OPT) $(TARGET_C_SJIS) $(TARGET_C) $(TARGET_GCC_LTO) $(TARGET_CLANG) $(TARGET_CLANG_LTO) $(TARGET_RCPP)
TARGETS+=$(OUTPUT_ASMS) $(OUTPUT_FUNCLIST)

# cppFriendsSpace.batで作るが、このMakefileで消去する
EXTERNAL_TARGETS=cppFriendsSpace

CASMFLAGS=-S -masm=intel
CASMFLAGS_87=-mfpmath=387 -mno-sse
CASMFLAGS_87_STORE=-mfpmath=387 -mno-sse -ffloat-store
CASMFLAGS_64=

SOURCE_MAIN=cppFriendsMain.cpp
SOURCE_FRIENDS=cppFriends.cpp
SOURCE_SAMPLE_1=cppFriendsSample1.cpp
SOURCE_SAMPLE_2=cppFriendsSample2.cpp
SOURCE_SAMPLE_ASM=cppFriendsSampleAsm.cpp
SOURCE_SAMPLE_SORT=cppFriendsSort.cpp
SOURCE_OPT=cppFriendsOpt.cpp
SOURCE_EXT=cppFriendsExt.cpp
SOURCE_SINGLETON=cppFriendsSingleton.cpp
SOURCE_THREAD=cppFriendsThread.cpp
SOURCE_CPP98=cppFriends98.cpp
SOURCE_CPP17=cppFriends17.cpp
SOURCE_SPACE=cppFriendsSpace.cpp
SOURCE_NET=cppFriendsNet.cpp
SOURCE_CLANG=cppFriendsClang.cpp
SOURCE_CLANG_EXT=cppFriendsClangExt.cpp
SOURCE_CLANG_TEST=cppFriendsClangTest.cpp
SOURCE_RCPP=cppFriendsRcpp.cpp

SOURCE_C_SJIS=cFriendsShiftJis.c
SOURCE_C=cFriends.c
SOURCE_C_EXT=cFriendsExt.c
SOURCE_ERROR=cppFriendsError.cpp
HEADERS_C=cFriends.h cFriendsCommon.h
INDENT_INPUT_SOURCE_H=cFriends.h
INDENT_INPUT_SOURCE_C=cFriends.c

SOURCE_RUBY_FILE_GENERATOR=cppFriendsFile.rb
SOURCE_RUBY_CASEWHEN=caseWhen.rb
SOURCE_RUBY_AMPM24=amPm24.rb
SOURCE_RUBY_SEATMAP=seatMap.rb
SOURCE_RUBY_SHUFFLE_LINES=shuffleLines.rb
SOURCE_RUBY_SHUFFLE_LINES_TEST=shuffleLinesTest.rb

LICENSE_FILE=LICENSE.txt
CPPFRIEND_BOT_TEXT=cppFriendsBot.txt

OBJ_MAIN=cppFriendsMain.o
OBJ_FRIENDS=cppFriends.o
OBJ_SAMPLE_1=cppFriendsSample1.o
OBJ_SAMPLE_2=cppFriendsSample2.o
OBJ_SAMPLE_ASM=cppFriendsSampleAsm.o
OBJ_SAMPLE_SORT=cppFriendsSort.o
OBJ_EXT=cppFriendsExt.o
OBJ_SINGLETON=cppFriendsSingleton.o
# スレッドとネットワークはMinGWでサポートしない
ifneq ($(BUILD_ON_MINGW),yes)
OBJ_OPT=cppFriendsOpt.o
OBJ_THREAD=cppFriendsThread.o
OBJ_NET=cppFriendsNet.o
OBJ_NO_OPT=cppFriendsOpt_no_opt.o
endif

OBJ_CPP98=cppFriends98.o
OBJ_CPP17=cppFriends17.o
OBJ_SPACE=cppFriendsSpace.o
OBJ_CLANG=cppFriendsClang.o
OBJ_CLANG_EXT=cppFriendsClangExt.o
OBJ_CLANG_TEST=cppFriendsClangTest.o
OBJ_CLANG_RCPP=cppFriendsRcpp.o

OBJ_NO_OPT_EXT=cppFriends_no_optExt.o
OBJ_MAIN_GCC_LTO=cppFriendsMain_gcc_lto.o
OBJ_CLANG_GCC_LTO=cppFriendsClang_gcc_lto.o
OBJ_CLANG_EXT_GCC_LTO=cppFriendsClangExt_gcc_lto.o
OBJ_CLANG_TEST_GCC_LTO=cppFriendsClangTest_gcc_lto.o

OBJS=$(OBJ_MAIN) $(OBJ_FRIENDS) $(OBJ_SAMPLE_1) $(OBJ_SAMPLE_2) $(OBJ_SAMPLE_ASM) $(OBJ_SAMPLE_SORT)
OBJS+=$(OBJ_OPT) $(OBJ_EXT) $(OBJ_SINGLETON) $(OBJ_THREAD) $(OBJ_CPP98) $(OBJ_SPACE)
ifeq ($(GCC_CPP17_AVAILABLE),1)
OBJS+=$(OBJ_CPP17)
endif

OBJS+=$(OBJ_NET)
OBJS+=$(OBJ_CLANG) $(OBJ_CLANG_EXT) $(OBJ_CLANG_TEST)
OBJS+=$(GTEST_OBJ)

OBJS_NO_OPT=$(OBJ_MAIN) $(OBJ_NO_OPT) $(OBJ_NO_OPT_EXT) $(GTEST_OBJ)
OBJS_GCC_LTO=$(OBJ_MAIN_GCC_LTO) $(OBJ_CLANG_GCC_LTO) $(OBJ_CLANG_EXT_GCC_LTO) $(OBJ_CLANG_TEST_GCC_LTO)
OBJS_GCC_LTO+=$(GTEST_OBJ_LTO)
OBJS_CLANG_RCPP=$(OBJ_CLANG_RCPP) $(GTEST_OBJ)

# BC -> .o -> .exeの途中
OBJ_ALL_IN_ONE_NO_LTO=cppFriendsClangAll_no_lto.o
OBJ_ALL_IN_ONE_LTO=cppFriendsClangAll_lto.o

BC_ALL=cppFriendsClangAll.bc
BC_ALL_LTO=cppFriendsClangAll_lto.bc
BC_OPT_LTO=cppFriendsClangOpt_lto.bc

BC_MAIN=cppFriendsMain.bc
BC_CLANG=cppFriendsClang.bc
BC_CLANG_EXT=cppFriendsClangExt.bc
BC_CLANG_TEST=cppFriendsClangTest.bc
BCS_CLANG=$(BC_MAIN) $(BC_CLANG) $(BC_CLANG_EXT) $(BC_CLANG_TEST) $(GTEST_BC)

BC_MAIN_LTO=cppFriendsMain_lto.bc
BC_CLANG_LTO=cppFriendsClang_lto.bc
BC_CLANG_EXT_LTO=cppFriendsClangExt_lto.bc
BC_CLANG_TEST_LTO=cppFriendsClangTest_lto.bc
BCS_CLANG_LTO=$(BC_MAIN_LTO) $(BC_CLANG_LTO) $(BC_CLANG_EXT_LTO) $(BC_CLANG_TEST_LTO) $(GTEST_BC_LTO)

BCS_OBJS=$(OBJ_ALL_IN_ONE_NO_LTO) $(OBJ_ALL_IN_ONE_LTO)
BCS_OBJS+=$(BC_ALL) $(BC_ALL_LTO) $(BC_OPT_LTO) $(BCS_CLANG) $(BCS_CLANG_LTO)
BCS_OBJS+=$(GTEST_BC) $(GTEST_BC_LTO)

VPATH=$(dir $(GTEST_SOURCE) $(GMOCK_SOURCE))

CPP=gcc
CXX=g++
CLANG=clang
CLANGXX=clang++
LLC=llc
LLVM_LINK=llvm-link
LLVM_OPT=opt
LD=g++
STRIP=strip
GREP?=grep
WC?=wc
RUBY?=ruby -Ku
CHCP_UTF8=chcp 65001
DETERMINE_FILE_TYPE=file

ifeq ($(BUILD_ON_MINGW),yes)
INCLUDES_GXX=$(addprefix -isystem ,$(ISYSTEM_MINGW_INCLUDE_DIR))
INCLUDES_CLANGXX=$(addprefix -isystem ,$(ISYSTEM_MINGW_CLANG_INCLUDE_DIRS))
endif

# http://d.hatena.ne.jp/higepon/20080430/1209525546
CFLAGS_WALL=-Wall -W -Wconversion -Wformat=2 -Wcast-qual -Wcast-align -Wwrite-strings -Wconversion -Wfloat-equal -Wpointer-arith -Wno-unused-parameter
CFLAGS_COMMON=-std=gnu11 -O2 $(CFLAGS_WALL)
GXX_CFLAGS=$(INCLUDES_GXX) $(CFLAGS_COMMON)
CLANGXX_CFLAGS=$(CLANG_TARGET) $(INCLUDES_CLANGXX) $(CFLAGS_COMMON)

CPPFLAGS_CPP98SPEC=-std=gnu++98
CPPFLAGS_CPP17SPEC=-std=gnu++17
CPPFLAGS_CPPSPEC=-std=gnu++14
CPPFLAGS_ARCH=-mavx2 -DCPPFRIENDS_AVX2
CPPFLAGS_COMMON=$(CFLAGS_WALL) $(GTEST_GMOCK_INCLUDE)
CPPFLAGS=$(CPPFLAGS_CPPSPEC) $(CPPFLAGS_COMMON) -O2

GXX_CPPFLAGS=$(INCLUDES_GXX) $(CPPFLAGS)
GXX_DEPEND_CPPFLAGS=$(GXX_CPPFLAGS)
GXX_DEPEND_CPPFLAGS_I=$(patsubst -isystem%, -I%, $(GXX_DEPEND_CPPFLAGS))
GXX_CPPFLAGS_NO_OPT=$(INCLUDES_GXX) $(CPPFLAGS_CPPSPEC) $(CPPFLAGS_COMMON) -g -O0 -DCPPFRIENDS_NO_OPTIMIZATION
GXX_CPPFLAGS_ERROR=$(INCLUDES_GXX) -std=gnu++1z $(CFLAGS_WALL) $(CPPFLAGS_COMMON) -O2
GXX_CPPFLAGS_LTO=$(INCLUDES_GXX) -flto -DCPPFRIENDS_ENABLE_LTO $(CPPFLAGS)
GXX_CPPFLAGS_RCPP=-DTESTING_FROM_MAIN
LDFLAGS_LTO=-Wno-attributes

CLANGXX_COMMON_FLAGS=$(INCLUDES_CLANGXX)
ifeq ($(BUILD_ON_MINGW),yes)
CLANGXX_COMMON_FLAGS+=$(CLANG_TARGET) -fno-exceptions -DBOOST_EXCEPTION_DISABLE=0 -DBOOST_NO_EXCEPTIONS=0
else
CLANGXX_COMMON_FLAGS+=-D__STRICT_ANSI__
endif

CLANGXX_DEPEND_CPPFLAGS=$(CLANGXX_COMMON_FLAGS) $(CPPFLAGS)
CLANGXX_DEPEND_CPPFLAGS_I=$(patsubst -isystem%, -I%, $(CLANGXX_DEPEND_CPPFLAGS))
CLANGXX_FLAGS=-S -emit-llvm $(CLANGXX_COMMON_FLAGS) $(CPPFLAGS)
CLANGXX_FLAGS_LTO=$(CLANGXX_FLAGS) $(CPPFLAGS_LTO)
LLCFLAGS=-filetype=obj
LLVM_OPT_FLAGS=-internalize -internalize-public-api-list=main,WinMain -O2

LIBPATH=
LDFLAGS=
LIBS=$(addsuffix $(BOOST_LIB_POSTFIX), -lboost_date_time -lboost_filesystem -lboost_locale -lboost_serialization -lboost_random -lboost_regex -lboost_system)
ifeq ($(BUILD_ON_MINGW),yes)
CLANGXX_LDFLAGS=-Wl,--allow-multiple-definition
CLANGXX_LIBS=-lsupc++
endif

INDENT_OPTIONS=--line-length10000 --dont-format-comments --dont-break-function-decl-args --dont-break-procedure-type --dont-line-up-parentheses --no-space-after-parentheses

RUBY_ONELINER_ASCII_ONLY='$$_.ascii_only? ? 0 : (puts $$. , $$_ ; abort)'

.PHONY: all test clean cprog force show
.SUFFIXES: .o .cpp .cc

all: $(TARGETS) force

$(PREPROCESSED_CLANG_TEST): $(SOURCE_CLANG_TEST)
	$(CXX) $(GXX_CPPFLAGS) -E -C -DONLY_PREPROCESSING -o $@ $<

$(TARGET): $(OBJS)
	$(LD) $(LIBPATH) -o $@ $^ $(LDFLAGS) $(LIBS)
	-find . -name "*.h" -o -name "*.hpp" -o -name "*.c" -o -name "*.cpp" | etags --language-force=C++ -L -
	./$@ --gtest_filter="-*DeathTest*"
	./$@ --gtest_filter="*DeathTest*"
	$(RM) $(OUTPUT_LOGS)
	./$@ --gtest_filter="TestCompoundStatement*" 1>$(OUTPUT_LOG_STDOUT) 2>$(OUTPUT_LOG_STDERR)
	$(GREP) -i "Printf in compound statements" $(OUTPUT_LOG_STDOUT)
	$(GREP) -i "Printf in compound statements" $(OUTPUT_LOG_STDERR)

$(TARGET_NO_OPT): $(OBJS_NO_OPT)
	$(LD) $(LIBPATH) -o $@ $^ $(LDFLAGS) $(LIBS)
	./$@ --gtest_filter="TestOpt*"

cprog: $(TARGET_C_SJIS) $(TARGET_C)

$(TARGET_C_SJIS): $(SOURCE_C_SJIS)
	$(CPP) -o $@ $<
	@./$@
	@./$@ | $(WC) | $(GREP) "  1  "
	-$(CPP) $(GXX_CFLAGS) -Werror -o $@ $<

$(TARGET_C): $(SOURCE_C) $(HEADERS_C)
	$(CPP) $(GXX_CFLAGS) $(CASMFLAGS_87) -o $@ $<
	@echo -e $(ECHO_START)Using x87 and compile with $(CASMFLAGS_87) $(ECHO_END)
	@./$@
	$(CPP) $(GXX_CFLAGS) $(CASMFLAGS_87_STORE) -o $@ $<
	@echo -e $(ECHO_START)Using x87 and compile with $(CASMFLAGS_87_STORE) $(ECHO_END)
	@./$@
	$(CPP) $(GXX_CFLAGS) $(CASMFLAGS_64) -o $@ $<
	@echo -e $(ECHO_START)Using x64 and SSE $(ECHO_END)
	@./$@

$(TARGET_GCC_LTO): $(OBJS_GCC_LTO)
	$(LD) $(GXX_CPPFLAGS_LTO) $(LDFLAGS_LTO) $(LIBPATH) -o $@ $^ $(LDFLAGS) $(LIBS)
	./$@

$(TARGET_CLANG): $(OBJ_ALL_IN_ONE_NO_LTO)
	$(CXX) -o $@ $< $(CLANGXX_LDFLAGS) $(CLANGXX_LIBS)
	$(STRIP) $@$(EXPLICIT_EXE_SUFFIX)
	./$@

$(TARGET_CLANG_LTO): $(OBJ_ALL_IN_ONE_LTO)
	$(CXX) $(GXX_CPPFLAGS_LTO) -o $@ $< $(CLANGXX_LIBS)
	$(STRIP) $@$(EXPLICIT_EXE_SUFFIX)
	./$@

$(TARGET_RCPP): $(OBJS_CLANG_RCPP)
	$(LD) $(LIBPATH) -o $@ $^ $(LDFLAGS) $(LIBS)
	@./$@

$(OBJ_CLANG_RCPP): $(SOURCE_RCPP)
	$(CPP) $(GXX_CPPFLAGS) $(GXX_CPPFLAGS_RCPP) -o $@ -c $<

$(OUTPUT_FUNCLIST): $(INDENT_INPUT_SOURCE_H) $(INDENT_INPUT_SOURCE_C)
	cat $(INDENT_INPUT_SOURCE_H) | expand | tr -d '\r' > $(OUTPUT_FUNCLIST_TEMP1_H)
	cat $(INDENT_INPUT_SOURCE_C) | expand | tr -d '\r' > $(OUTPUT_FUNCLIST_TEMP1_C)
	indent $(INDENT_OPTIONS) -o $(OUTPUT_FUNCLIST_TEMP2_H) $(OUTPUT_FUNCLIST_TEMP1_H)
	indent $(INDENT_OPTIONS) -o $(OUTPUT_FUNCLIST_TEMP2_C) $(OUTPUT_FUNCLIST_TEMP1_C)
	ctags -x --c-kinds=f --fields=+S $(OUTPUT_FUNCLIST_TEMP_SRCS) | sed -e 's/  */ /g' | cut -d' ' -f5- | tee $(OUTPUT_FUNCLIST)

$(OUTPUT_ASM87_C): $(SOURCE_C)
	$(CPP) $(GXX_CFLAGS) $(CASMFLAGS) $(CASMFLAGS_87) -o $@ $<

$(OUTPUT_ASM87_STORE_C): $(SOURCE_C)
	$(CPP) $(GXX_CFLAGS) $(CASMFLAGS) $(CASMFLAGS_87_STORE) -o $@ $<

$(OUTPUT_ASM64_C): $(SOURCE_C)
	$(CPP) $(GXX_CFLAGS) $(CASMFLAGS) $(CASMFLAGS_64) -o $@ $<

$(OUTPUT_ASM64_CLANG): $(SOURCE_C)
	$(CLANG) $(CLANGXX_CFLAGS) $(CASMFLAGS) $(CASMFLAGS_64) -o $@ $<

$(OUTPUT_ASM_C_EXT1): $(SOURCE_C_EXT)
	$(CPP) $(GXX_CFLAGS) $(CASMFLAGS) -o $@ $<

$(OUTPUT_ASM_C_EXT2): $(SOURCE_C_EXT)
	$(CPP) $(GXX_CFLAGS) $(CASMFLAGS) -DCPPFRIENDS_SIDE_EFFECT -o $@ $<

$(OUTPUT_ASM_CPP_CLANG): $(SOURCE_CLANG)
	$(CLANGXX) $(CLANGXX_COMMON_FLAGS) $(CPPFLAGS) $(CASMFLAGS) -o $@ $<

$(OUTPUT_ASM_CPP_GCC): $(SOURCE_CLANG)
	$(CXX) $(GXX_CPPFLAGS) $(CASMFLAGS) $(CASMFLAGS_64) -o $@ $<

$(OUTPUT_ASM_SINGLETON_11): $(SOURCE_SINGLETON)
	$(CXX) $(GXX_CPPFLAGS) $(CASMFLAGS) -o $@ $<

$(OUTPUT_ASM_SINGLETON_DEFAULT): $(SOURCE_SINGLETON)
	$(CXX) $(GXX_CPPFLAGS) $(CPPFLAGS_CPP98SPEC)  $(CASMFLAGS) -o $@ $<

$(OUTPUT_ASM_SINGLETON_NO): $(SOURCE_SINGLETON)
	$(CXX) $(GXX_CPPFLAGS) $(CPPFLAGS_CPP98SPEC) -fno-threadsafe-statics $(CASMFLAGS) -o $@ $<

force : $(SOURCE_ERROR)
	$(RUBY) $(SOURCE_RUBY_CASEWHEN)
	$(RUBY) $(SOURCE_RUBY_AMPM24)
	$(RUBY) $(SOURCE_RUBY_SEATMAP) | $(GREP) -i "100% passed"
	$(RUBY) -ne $(RUBY_ONELINER_ASCII_ONLY) $(LICENSE_FILE)
	$(RUBY) -ne $(RUBY_ONELINER_ASCII_ONLY) $(SOURCE_FRIENDS) ; test $$? -ne 0
	$(DETERMINE_FILE_TYPE) $(LICENSE_FILE) | $(GREP) -i "ASCII text"
	$(DETERMINE_FILE_TYPE) $(SOURCE_FRIENDS) | $(GREP) -i "UTF-8 Unicode text"
	$(DETERMINE_FILE_TYPE) $(SOURCE_C_SJIS)| $(GREP) -i "Non-ISO extended-ASCII"
ifeq ($(BUILD_ON_MINGW),yes)
	$(CHCP_UTF8)
endif
	$(RUBY) $(SOURCE_RUBY_SHUFFLE_LINES_TEST)
	bash -c "comm -3 <(sort $(CPPFRIEND_BOT_TEXT)) <($(RUBY) $(SOURCE_RUBY_SHUFFLE_LINES) -i $(CPPFRIEND_BOT_TEXT) | sort)" | wc | $(GREP) "  0 "
	$(GREP) -v --quiet "vtable" $(CPPFRIEND_BOT_TEXT) ; test $$? -eq 0
	$(GREP) --quiet "fennec" $(CPPFRIEND_BOT_TEXT) ; test $$? -eq 1
	-$(CXX) $(CPPFLAGS_ERROR) -c $<

$(OBJ_MAIN): $(SOURCE_MAIN)
	$(CXX) $(GXX_CPPFLAGS) -o $@ -c $<

$(OBJ_FRIENDS): $(SOURCE_FRIENDS)
	$(CXX) $(GXX_CPPFLAGS) -o $@ -c $<

$(OUTPUT_SAMPLE_2_DEPEND_GCC): $(SOURCE_SAMPLE_2)
	$(CXX) -MM -MG $(GXX_DEPEND_CPPFLAGS) $< | sed 's|\(.*\)\.o|$(OBJ_DIR)/\1\.o|' > $@

$(OUTPUT_SAMPLE_2_DEPEND_GCC_I): $(SOURCE_SAMPLE_2)
	$(CXX) -MM -MG $(GXX_DEPEND_CPPFLAGS_I) $< | sed 's|\(.*\)\.o|$(OBJ_DIR)/\1\.o|' > $@

$(OUTPUT_SAMPLE_2_DEPEND_CLANG): $(SOURCE_SAMPLE_2)
	$(CLANGXX) -MM -MG $(CLANGXX_DEPEND_CPPFLAGS) $< | sed 's|\(.*\)\.o|$(OBJ_DIR)/\1\.o|' > $@

$(OUTPUT_SAMPLE_2_DEPEND_CLANG_I): $(SOURCE_SAMPLE_2)
	$(CLANGXX) -MM -MG $(CLANGXX_DEPEND_CPPFLAGS_I) $< | sed 's|\(.*\)\.o|$(OBJ_DIR)/\1\.o|' > $@

$(OBJ_SAMPLE_1): $(SOURCE_SAMPLE_1)
	$(CXX) $(GXX_CPPFLAGS) -o $@ -c $<

$(OBJ_SAMPLE_2): $(SOURCE_SAMPLE_2) $(OUTPUT_GENERATED_HEADER)
	$(CXX) $(GXX_CPPFLAGS) -o $@ -c $<
ifneq ($(BUILD_ON_MINGW),yes)
	$(CXX) -E -H $(GXX_CPPFLAGS) $< 1> /dev/null 2> $(OUTPUT_SAMPLE_2_HEADERS)
	$(WC) $(OUTPUT_SAMPLE_2_HEADERS)
endif

$(OUTPUT_GENERATED_HEADER):
	$(RUBY) $(SOURCE_RUBY_FILE_GENERATOR) create $(OUTPUT_GENERATED_HEADER)

$(OBJ_SAMPLE_ASM): $(SOURCE_SAMPLE_ASM)
	$(CXX) $(GXX_CPPFLAGS) $(CPPFLAGS_ARCH) -o $@ -c $<

$(OBJ_SAMPLE_SORT): $(SOURCE_SAMPLE_SORT)
	$(CXX) $(GXX_CPPFLAGS) -o $@ -c $<

$(OBJ_OPT): $(SOURCE_OPT)
	$(CXX) $(GXX_CPPFLAGS) -o $@ -c $<

$(OBJ_EXT): $(SOURCE_EXT)
	$(CXX) $(GXX_CPPFLAGS) -o $@ -c $<

$(OBJ_THREAD): $(SOURCE_THREAD)
	$(CXX) $(GXX_CPPFLAGS) -o $@ -c $<

$(OBJ_CPP98): $(SOURCE_CPP98)
	$(CXX) $(INCLUDES_GXX) $(CPPFLAGS_COMMON) $(CPPFLAGS_CPP98SPEC) -O2 -o $@ -c $<

$(OBJ_CPP17): $(SOURCE_CPP17)
	$(CXX) $(INCLUDES_GXX) $(CPPFLAGS_COMMON) $(CPPFLAGS_CPP17SPEC) -O2 -o $@ -c $<

$(OBJ_SPACE): $(SOURCE_SPACE)
	$(CXX) $(GXX_CPPFLAGS) -o $@ -c $<

$(OBJ_NET): $(SOURCE_NET)
	$(CXX) $(GXX_CPPFLAGS) -o $@ -c $<

$(OBJ_CLANG): $(SOURCE_CLANG)
	$(CXX) $(GXX_CPPFLAGS) -o $@ -c $<

$(OBJ_CLANG_EXT): $(SOURCE_CLANG_EXT)
	$(CXX) $(GXX_CPPFLAGS) -o $@ -c $<

$(OBJ_CLANG_TEST): $(SOURCE_CLANG_TEST)
	$(CXX) $(GXX_CPPFLAGS) -o $@ -c $<

$(OBJ_NO_OPT): $(SOURCE_OPT)
	$(CXX) $(GXX_CPPFLAGS_NO_OPT) -o $@ -c $<

$(OBJ_NO_OPT_EXT): $(SOURCE_EXT)
	$(CXX) $(GXX_CPPFLAGS_NO_OPT) -o $@ -c $<

$(OBJ_MAIN_GCC_LTO): $(SOURCE_MAIN)
	$(CXX) $(GXX_CPPFLAGS_LTO) -o $@ -c $<

$(OBJ_CLANG_GCC_LTO): $(SOURCE_CLANG)
	$(CXX) $(GXX_CPPFLAGS_LTO) -o $@ -c $<

$(OBJ_CLANG_EXT_GCC_LTO): $(SOURCE_CLANG_EXT)
	$(CXX) $(GXX_CPPFLAGS_LTO) -o $@ -c $<

$(OBJ_CLANG_TEST_GCC_LTO): $(SOURCE_CLANG_TEST)
	$(CXX) $(GXX_CPPFLAGS_LTO) -o $@ -c $<

$(GTEST_OBJ): $(GTEST_SOURCE)
	$(CXX) $(GXX_CPPFLAGS) -o $@ -c $<

$(GTEST_OBJ_LTO): $(GTEST_SOURCE)
	$(CXX) $(GXX_CPPFLAGS_LTO) -o $@ -c $<

$(OBJ_ALL_IN_ONE_NO_LTO) : $(BC_ALL)
	$(LLC) $(LLCFLAGS) -o $@ $<

$(OBJ_ALL_IN_ONE_LTO) : $(BC_OPT_LTO)
	$(LLC) $(LLCFLAGS) -o $@ $<

$(BC_ALL): $(BCS_CLANG)
	$(LLVM_LINK) -o $@ $^

$(BC_ALL_LTO): $(BCS_CLANG_LTO)
	$(LLVM_LINK) -o $@ $^

$(BC_OPT_LTO): $(BC_ALL_LTO)
	$(LLVM_OPT) $(LLVM_OPT_FLAGS) -o $@ $<

$(BC_MAIN): $(SOURCE_MAIN)
	$(CLANGXX) $(CLANGXX_FLAGS) -o $@ $<

$(BC_CLANG): $(SOURCE_CLANG)
	$(CLANGXX) $(CLANGXX_FLAGS) -o $@ $<

$(BC_CLANG_EXT): $(SOURCE_CLANG_EXT)
	$(CLANGXX) $(CLANGXX_FLAGS) -o $@ $<

$(BC_CLANG_TEST): $(SOURCE_CLANG_TEST)
	$(CLANGXX) $(CLANGXX_FLAGS) -o $@ $<

$(BC_MAIN_LTO): $(SOURCE_MAIN)
	$(CLANGXX) $(CLANGXX_FLAGS_LTO) -o $@ $<

$(BC_CLANG_LTO): $(SOURCE_CLANG)
	$(CLANGXX) $(CLANGXX_FLAGS_LTO) -o $@ $<

$(BC_CLANG_EXT_LTO): $(SOURCE_CLANG_EXT)
	$(CLANGXX) $(CLANGXX_FLAGS_LTO) -o $@ $<

$(BC_CLANG_TEST_LTO): $(SOURCE_CLANG_TEST)
	$(CLANGXX) $(CLANGXX_FLAGS) -o $@ $<

$(GTEST_BC): $(GTEST_SOURCE)
	$(CLANGXX) $(CLANGXX_FLAGS) -o $@ $<

$(GTEST_BC_LTO): $(GTEST_SOURCE)
	$(CLANGXX) $(CLANGXX_FLAGS_LTO) -o $@ $<

test: $(TARGET)
	./$(TARGET)

clean:
	$(RM) $(TARGETS) $(EXTERNAL_TARGETS) $(OBJS) $(OBJS_NO_OPT) $(OBJS_GCC_LTO) $(OBJS_CLANG_RCPP) $(BCS_OBJS) $(OUTPUT_ASMS) $(OUTPUT_GENERATED_HEADER) $(OUTPUT_FUNCLIST) $(OUTPUT_PREPROCESSED_FILES) $(OUTPUT_FUNCLIST_TEMPS) $(OUTPUT_LOGS) ./*.o ./*.bc ./*.s ./*.d
	$(RUBY) $(SOURCE_RUBY_FILE_GENERATOR) delete

show:
	$(foreach v, $(.VARIABLES), $(info $(v) = $($(v))))
