# コンソール出力に色を付ける
ECHO_START="\e[104m
ECHO_END=\e[0m"

# Google Test / Google Mockがあるディレクトリ
GTEST_GMOCK_TOP_DIR=$(HOME)/googletest
GTEST_TOP_DIR=$(GTEST_GMOCK_TOP_DIR)/googletest
GMOCK_TOP_DIR=$(GTEST_GMOCK_TOP_DIR)/googlemock
GTEST_GMOCK_INCLUDE=$(addprefix -isystem, $(GTEST_TOP_DIR)/include $(GTEST_TOP_DIR) $(GMOCK_TOP_DIR)/include $(GMOCK_TOP_DIR))
GTEST_SOURCE=$(GTEST_TOP_DIR)/src/gtest-all.cc
GMOCK_SOURCE=$(GMOCK_TOP_DIR)/src/gmock-all.cc

GTEST_OBJ=$(patsubst %.cc, %.o, $(notdir $(GTEST_SOURCE)))
GTEST_OBJ_LTO=$(patsubst %.cc, %_lto.o, $(notdir $(GTEST_SOURCE)))
GTEST_BC=$(patsubst %.cc, %.bc, $(notdir $(GTEST_SOURCE)))
GTEST_BC_LTO=$(patsubst %.cc, %_lto.bc, $(notdir $(GTEST_SOURCE)))

TARGET=cppFriends
TARGET_NO_OPT=cppFriends_no_opt
TARGET_C_SJIS=cFriendsShiftJis
TARGET_C=cFriends
TARGET_GCC_LTO=cppFriends_gcc_lto
TARGET_CLANG=cppFriendsClang
TARGET_CLANG_LTO=cppFriendsClang_lto
OUTPUT_ASM87_C=cFriends87.s
OUTPUT_ASM87_STORE_C=cFriends87-store.s
OUTPUT_ASM64_C=cFriends64.s
OUTPUT_ASM64_CLANG=cFriends64_clang.s
OUTPUT_ASM_C_EXT1=cFriendsNoSideEffect.s
OUTPUT_ASM_C_EXT2=cFriendsSideEffect.s
OUTPUT_ASM_CPP_CLANG=cppFriendsClang.s
OUTPUT_ASMS=$(OUTPUT_ASM87_C) $(OUTPUT_ASM87_STORE_C) $(OUTPUT_ASM64_C) $(OUTPUT_ASM64_CLANG) $(OUTPUT_ASM_C_EXT1) $(OUTPUT_ASM_C_EXT2) $(OUTPUT_ASM_CPP_CLANG)

OUTPUT_FUNCLIST_TEMP1_H=__cFriends_nocr1__.h
OUTPUT_FUNCLIST_TEMP1_C=__cFriends_nocr1__.c
OUTPUT_FUNCLIST_TEMP2_H=__cFriends_nocr2__.h
OUTPUT_FUNCLIST_TEMP2_C=__cFriends_nocr2__.c
OUTPUT_FUNCLIST_TEMP_SRCS=$(OUTPUT_FUNCLIST_TEMP2_H) $(OUTPUT_FUNCLIST_TEMP2_C)
OUTPUT_FUNCLIST_TEMPS=$(OUTPUT_FUNCLIST_TEMP1_H) $(OUTPUT_FUNCLIST_TEMP1_C) $(OUTPUT_FUNCLIST_TEMP_SRCS)
OUTPUT_FUNCLIST=cFriends_func.txt

TARGETS=$(TARGET) $(TARGET_NO_OPT) $(TARGET_C_SJIS) $(TARGET_C) $(TARGET_GCC_LTO) $(TARGET_CLANG) $(TARGET_CLANG_LTO)
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
SOURCE_OPT=cppFriendsOpt.cpp
SOURCE_EXT=cppFriendsExt.cpp
SOURCE_THREAD=cppFriendsThread.cpp
SOURCE_CPP98=cppFriends98.cpp
SOURCE_SPACE=cppFriendsSpace.cpp
SOURCE_NET=cppFriendsNet.cpp
SOURCE_CLANG=cppFriendsClang.cpp
SOURCE_CLANG_EXT=cppFriendsClangExt.cpp
SOURCE_CLANG_TEST=cppFriendsClangTest.cpp

SOURCE_C_SJIS=cFriendsShiftJis.c
SOURCE_C=cFriends.c
SOURCE_C_EXT=cFriendsExt.c
SOURCE_ERROR=cppFriendsError.cpp
INDENT_INPUT_SOURCE_H=cFriends.h
INDENT_INPUT_SOURCE_C=cFriends.c

SOURCE_RUBY_CASEWHEN=caseWhen.rb
SOURCE_RUBY_SEATMAP=seatMap.rb

LICENSE_FILE=LICENSE.txt

OBJ_MAIN=cppFriendsMain.o
OBJ_FRIENDS=cppFriends.o
OBJ_SAMPLE_1=cppFriendsSample1.o
OBJ_SAMPLE_2=cppFriendsSample2.o
OBJ_SAMPLE_ASM=cppFriendsSampleAsm.o
OBJ_OPT=cppFriendsOpt.o
OBJ_EXT=cppFriendsExt.o
OBJ_THREAD=cppFriendsThread.o
OBJ_CPP98=cppFriends98.o
OBJ_SPACE=cppFriendsSpace.o
OBJ_NET=cppFriendsNet.o
OBJ_CLANG=cppFriendsClang.o
OBJ_CLANG_EXT=cppFriendsClangExt.o
OBJ_CLANG_TEST=cppFriendsClangTest.o

OBJ_NO_OPT=cppFriendsOpt_no_opt.o
OBJ_NO_OPT_EXT=cppFriends_no_optExt.o
OBJ_MAIN_GCC_LTO=cppFriendsMain_gcc_lto.o
OBJ_CLANG_GCC_LTO=cppFriendsClang_gcc_lto.o
OBJ_CLANG_EXT_GCC_LTO=cppFriendsClangExt_gcc_lto.o
OBJ_CLANG_TEST_GCC_LTO=cppFriendsClangTest_gcc_lto.o

OBJS=$(OBJ_MAIN) $(OBJ_FRIENDS) $(OBJ_SAMPLE_1) $(OBJ_SAMPLE_2) $(OBJ_SAMPLE_ASM)
OBJS+=$(OBJ_OPT) $(OBJ_EXT) $(OBJ_THREAD) $(OBJ_CPP98) $(OBJ_SPACE)
OBJS+=$(OBJ_NET)
OBJS+=$(OBJ_CLANG) $(OBJ_CLANG_EXT) $(OBJ_CLANG_TEST)
OBJS+=$(GTEST_OBJ)

OBJS_NO_OPT=$(OBJ_MAIN) $(OBJ_NO_OPT) $(OBJ_NO_OPT_EXT) $(GTEST_OBJ)
OBJS_GCC_LTO=$(OBJ_MAIN_GCC_LTO) $(OBJ_CLANG_GCC_LTO) $(OBJ_CLANG_EXT_GCC_LTO) $(OBJ_CLANG_TEST_GCC_LTO)
OBJS_GCC_LTO+=$(GTEST_OBJ_LTO)

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
RUBY?=ruby
DETERMINE_FILE_TYPE=file

CFLAGS=-std=gnu11 -O2 -Wall
CPPFLAGS_CPPSPEC=-std=gnu++14
CPPFLAGS_COMMON=-Wall $(GTEST_GMOCK_INCLUDE)
CPPFLAGS=$(CPPFLAGS_CPPSPEC) $(CPPFLAGS_COMMON) -O2
CPPFLAGS_NO_OPT=$(CPPFLAGS_CPPSPEC) $(CPPFLAGS_COMMON) -g -O0 -DCPPFRIENDS_NO_OPTIMIZATION
CPPFLAGS_ERROR=-std=gnu++1z -Wall $(CPPFLAGS_COMMON) -O2

CPPFLAGS_LTO=-flto -DCPPFRIENDS_ENABLE_LTO
LDFLAGS_LTO=-Wno-attributes
CLANGXX_COMMON_FLAGS=-D__STRICT_ANSI__
CLANGXXFLAGS=-S -emit-llvm $(CLANGXX_COMMON_FLAGS)
CLANGXXFLAGS_LTO=$(CPPFLAGS_LTO) $(CLANGXXFLAGS)
LLCFLAGS=-filetype=obj
LLVM_OPT_FLAGS=-internalize -internalize-public-api-list=main,WinMain -O2

LIBPATH=
LDFLAGS=
LIBS=-lboost_date_time -lboost_locale -lboost_serialization -lboost_random -lboost_regex -lboost_system

INDENT_OPTIONS=--line-length10000 --dont-format-comments --dont-break-function-decl-args --dont-break-procedure-type --dont-line-up-parentheses --no-space-after-parentheses

RUBY_ONELINER_ASCII_ONLY='$$_.ascii_only? ? 0 : (puts $$. , $$_ ; abort)'

.PHONY: all test clean cprog force show
.SUFFIXES: .o .cpp .cc

all: $(TARGETS) force

$(TARGET): $(OBJS)
	$(LD) $(LIBPATH) -o $@ $^ $(LDFLAGS) $(LIBS)
	-find . -name "*.h" -o -name "*.hpp" -o -name "*.c" -o -name "*.cpp" | etags --language-force=C++ -L -
	./$@

$(TARGET_NO_OPT): $(OBJS_NO_OPT)
	$(LD) $(LIBPATH) -o $@ $^ $(LDFLAGS) $(LIBS)
	./$@ --gtest_filter="TestMyCounter*:TestShifter*"

cprog: $(TARGET_C_SJIS) $(TARGET_C)

$(TARGET_C_SJIS): $(SOURCE_C_SJIS)
	$(CPP) -o $@ $<
	@./$@
	@./$@ | $(WC) | $(GREP) "  1  "
	-$(CPP) $(CFLAGS) -Werror -o $@ $<

$(TARGET_C): $(SOURCE_C)
	$(CPP) $(CFLAGS) $(CASMFLAGS_87) -o $@ $<
	@echo -e $(ECHO_START)Using x87 and compile with $(CASMFLAGS_87) $(ECHO_END)
	@./$@
	$(CPP) $(CFLAGS) $(CASMFLAGS_87_STORE) -o $@ $<
	@echo -e $(ECHO_START)Using x87 and compile with $(CASMFLAGS_87_STORE) $(ECHO_END)
	@./$@
	$(CPP) $(CFLAGS) $(CASMFLAGS_64) -o $@ $<
	@echo -e $(ECHO_START)Using x64 and SSE $(ECHO_END)
	@./$@

$(TARGET_GCC_LTO): $(OBJS_GCC_LTO)
	$(LD) $(CPPFLAGS_LTO) $(LDFLAGS_LTO) $(LIBPATH) -o $@ $^ $(LDFLAGS) $(LIBS)
	./$@

$(TARGET_CLANG): $(OBJ_ALL_IN_ONE_NO_LTO)
	$(CXX) -o $@ $<
	$(STRIP) $@
	./$@

$(TARGET_CLANG_LTO): $(OBJ_ALL_IN_ONE_LTO)
	$(CXX) $(CPPFLAGS_LTO) -o $@ $<
	$(STRIP) $@
	./$@

$(OUTPUT_FUNCLIST): $(INDENT_INPUT_SOURCE_H) $(INDENT_INPUT_SOURCE_C)
	cat $(INDENT_INPUT_SOURCE_H) | expand | tr -d '\r' > $(OUTPUT_FUNCLIST_TEMP1_H)
	cat $(INDENT_INPUT_SOURCE_C) | expand | tr -d '\r' > $(OUTPUT_FUNCLIST_TEMP1_C)
	indent $(INDENT_OPTIONS) -o $(OUTPUT_FUNCLIST_TEMP2_H) $(OUTPUT_FUNCLIST_TEMP1_H)
	indent $(INDENT_OPTIONS) -o $(OUTPUT_FUNCLIST_TEMP2_C) $(OUTPUT_FUNCLIST_TEMP1_C)
	ctags -x --c-kinds=f --fields=+S $(OUTPUT_FUNCLIST_TEMP_SRCS) | sed -e 's/  */ /g' | cut -d' ' -f5- | tee $(OUTPUT_FUNCLIST)

$(OUTPUT_ASM87_C): $(SOURCE_C)
	$(CPP) $(CFLAGS) $(CASMFLAGS) $(CASMFLAGS_87) -o $@ $<

$(OUTPUT_ASM87_STORE_C): $(SOURCE_C)
	$(CPP) $(CFLAGS) $(CASMFLAGS) $(CASMFLAGS_87_STORE) -o $@ $<

$(OUTPUT_ASM64_C): $(SOURCE_C)
	$(CPP) $(CFLAGS) $(CASMFLAGS) $(CASMFLAGS_64) -o $@ $<

$(OUTPUT_ASM64_CLANG): $(SOURCE_C)
	$(CLANG) $(CLANGXX_COMMON_FLAGS) $(CFLAGS) $(CASMFLAGS) $(CASMFLAGS_64) -o $@ $<

$(OUTPUT_ASM_C_EXT1): $(SOURCE_C_EXT)
	$(CPP) $(CFLAGS) $(CASMFLAGS) -o $@ $<

$(OUTPUT_ASM_C_EXT2): $(SOURCE_C_EXT)
	$(CPP) $(CFLAGS) $(CASMFLAGS) -DCPPFRIENDS_SIDE_EFFECT -o $@ $<

$(OUTPUT_ASM_CPP_CLANG): $(SOURCE_CLANG)
	$(CLANGXX) $(CLANGXX_COMMON_FLAGS) $(CPPFLAGS) $(CASMFLAGS) -o $@ $<

force : $(SOURCE_ERROR)
	$(RUBY) $(SOURCE_RUBY_CASEWHEN)
	$(RUBY) $(SOURCE_RUBY_SEATMAP) | grep -i "100% passed"
	$(RUBY) -ne $(RUBY_ONELINER_ASCII_ONLY) $(LICENSE_FILE)
	$(RUBY) -ne $(RUBY_ONELINER_ASCII_ONLY) $(SOURCE_FRIENDS) ; test $$? -ne 0
	$(DETERMINE_FILE_TYPE) $(LICENSE_FILE) | $(GREP) -i "ASCII text"
	$(DETERMINE_FILE_TYPE) $(SOURCE_FRIENDS) | $(GREP) -i "UTF-8 Unicode text"
	$(DETERMINE_FILE_TYPE) $(SOURCE_C_SJIS)| $(GREP) -i "Non-ISO extended-ASCII"

	-$(CXX) $(CPPFLAGS_ERROR) -c $<

$(OBJ_MAIN): $(SOURCE_MAIN)
	$(CXX) $(CPPFLAGS) -o $@ -c $<

$(OBJ_FRIENDS): $(SOURCE_FRIENDS)
	$(CXX) $(CPPFLAGS) -o $@ -c $<

$(OBJ_SAMPLE_1): $(SOURCE_SAMPLE_1)
	$(CXX) $(CPPFLAGS) -o $@ -c $<

$(OBJ_SAMPLE_2): $(SOURCE_SAMPLE_2)
	$(CXX) $(CPPFLAGS) -o $@ -c $<

$(OBJ_SAMPLE_ASM): $(SOURCE_SAMPLE_ASM)
	$(CXX) $(CPPFLAGS) -o $@ -c $<

$(OBJ_OPT): $(SOURCE_OPT)
	$(CXX) $(CPPFLAGS) -o $@ -c $<

$(OBJ_EXT): $(SOURCE_EXT)
	$(CXX) $(CPPFLAGS) -o $@ -c $<

$(OBJ_THREAD): $(SOURCE_THREAD)
	$(CXX) $(CPPFLAGS) -o $@ -c $<

$(OBJ_CPP98): $(SOURCE_CPP98)
	$(CXX) $(CPPFLAGS_COMMON) -O2 -o $@ -c $<

$(OBJ_SPACE): $(SOURCE_SPACE)
	$(CXX) $(CPPFLAGS) -o $@ -c $<

$(OBJ_NET): $(SOURCE_NET)
	$(CXX) $(CPPFLAGS) -o $@ -c $<

$(OBJ_CLANG): $(SOURCE_CLANG)
	$(CXX) $(CPPFLAGS) -o $@ -c $<

$(OBJ_CLANG_EXT): $(SOURCE_CLANG_EXT)
	$(CXX) $(CPPFLAGS) -o $@ -c $<

$(OBJ_CLANG_TEST): $(SOURCE_CLANG_TEST)
	$(CXX) $(CPPFLAGS) -o $@ -c $<

$(OBJ_NO_OPT): $(SOURCE_OPT)
	$(CXX) $(CPPFLAGS_NO_OPT) -o $@ -c $<

$(OBJ_NO_OPT_EXT): $(SOURCE_EXT)
	$(CXX) $(CPPFLAGS_NO_OPT) -o $@ -c $<

$(OBJ_MAIN_GCC_LTO): $(SOURCE_MAIN)
	$(CXX) $(CPPFLAGS_LTO) $(CPPFLAGS) -o $@ -c $<

$(OBJ_CLANG_GCC_LTO): $(SOURCE_CLANG)
	$(CXX) $(CPPFLAGS_LTO) $(CPPFLAGS) -o $@ -c $<

$(OBJ_CLANG_EXT_GCC_LTO): $(SOURCE_CLANG_EXT)
	$(CXX) $(CPPFLAGS_LTO) $(CPPFLAGS) -o $@ -c $<

$(OBJ_CLANG_TEST_GCC_LTO): $(SOURCE_CLANG_TEST)
	$(CXX) $(CPPFLAGS_LTO) $(CPPFLAGS) -o $@ -c $<

$(GTEST_OBJ): $(GTEST_SOURCE)
	$(CXX) $(CPPFLAGS) -o $@ -c $<

$(GTEST_OBJ_LTO): $(GTEST_SOURCE)
	$(CXX) $(CPPFLAGS_LTO) $(CPPFLAGS) -o $@ -c $<

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
	$(CLANGXX) $(CLANGXXFLAGS) $(CPPFLAGS) -o $@ $<

$(BC_CLANG): $(SOURCE_CLANG)
	$(CLANGXX) $(CLANGXXFLAGS) $(CPPFLAGS) -o $@ $<

$(BC_CLANG_EXT): $(SOURCE_CLANG_EXT)
	$(CLANGXX) $(CLANGXXFLAGS) $(CPPFLAGS) -o $@ $<

$(BC_CLANG_TEST): $(SOURCE_CLANG_TEST)
	$(CLANGXX) $(CLANGXXFLAGS) $(CPPFLAGS) -o $@ $<

$(BC_MAIN_LTO): $(SOURCE_MAIN)
	$(CLANGXX) $(CLANGXXFLAGS_LTO) $(CPPFLAGS) -o $@ $<

$(BC_CLANG_LTO): $(SOURCE_CLANG)
	$(CLANGXX) $(CLANGXXFLAGS_LTO) $(CPPFLAGS) -o $@ $<

$(BC_CLANG_EXT_LTO): $(SOURCE_CLANG_EXT)
	$(CLANGXX) $(CLANGXXFLAGS_LTO) $(CPPFLAGS) -o $@ $<

$(BC_CLANG_TEST_LTO): $(SOURCE_CLANG_TEST)
	$(CLANGXX) $(CLANGXXFLAGS) $(CPPFLAGS) -o $@ $<

$(GTEST_BC): $(GTEST_SOURCE)
	$(CLANGXX) $(CLANGXXFLAGS) $(CPPFLAGS) -o $@ $<

$(GTEST_BC_LTO): $(GTEST_SOURCE)
	$(CLANGXX) $(CLANGXXFLAGS_LTO) $(CPPFLAGS) -o $@ $<

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGETS) $(EXTERNAL_TARGETS) $(OBJS) $(OBJS_NO_OPT) $(OBJS_GCC_LTO) $(BCS_OBJS) $(OUTPUT_ASMS) $(OUTPUT_FUNCLIST) $(OUTPUT_FUNCLIST_TEMPS) ./*.o ./*.bc ./*.s

show:
	$(foreach v, $(.VARIABLES), $(info $(v) = $($(v))))
