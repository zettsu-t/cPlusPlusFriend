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
GMOCK_OBJ=$(patsubst %.cc, %.o, $(notdir $(GMOCK_SOURCE)))

TARGET=cppFriends
TARGET_NO_OPT=cppFriends_no_opt
TARGET_C_SJIS=cFriendsShiftJis
TARGET_C=cFriends
OUTPUT_ASM87_C=cFriends87.s
OUTPUT_ASM87_STORE_C=cFriends87-store.s
OUTPUT_ASM64_C=cFriends64.s
OUTPUT_ASM_C_EXT1=cFriendsNoSideEffect.s
OUTPUT_ASM_C_EXT2=cFriendsSideEffect.s
OUTPUT_ASM_CPP_CLANG=cppFriendsClang.s
OUTPUT_ASMS=$(OUTPUT_ASM87_C) $(OUTPUT_ASM87_STORE_C) $(OUTPUT_ASM64_C) $(OUTPUT_ASM_C_EXT1) $(OUTPUT_ASM_C_EXT2) $(OUTPUT_ASM_CPP_CLANG)

OUTPUT_FUNCLIST_TEMP1_H=__cFriends_nocr1__.h
OUTPUT_FUNCLIST_TEMP1_C=__cFriends_nocr1__.c
OUTPUT_FUNCLIST_TEMP2_H=__cFriends_nocr2__.h
OUTPUT_FUNCLIST_TEMP2_C=__cFriends_nocr2__.c
OUTPUT_FUNCLIST_TEMP_SRCS=$(OUTPUT_FUNCLIST_TEMP2_H) $(OUTPUT_FUNCLIST_TEMP2_C)
OUTPUT_FUNCLIST_TEMPS=$(OUTPUT_FUNCLIST_TEMP1_H) $(OUTPUT_FUNCLIST_TEMP1_C) $(OUTPUT_FUNCLIST_TEMP_SRCS)
OUTPUT_FUNCLIST=cFriends_func.txt

TARGETS=$(TARGET) $(TARGET_NO_OPT) $(TARGET_C_SJIS) $(TARGET_C) $(OUTPUT_ASMS) $(OUTPUT_FUNCLIST)

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
SOURCE_SPACE=cppFriendsSpace.cpp
SOURCE_CLANG=cppFriendsClang.cpp
SOURCE_CLANG_EXT=cppFriendsClangExt.cpp
SOURCE_CPP98=cppFriends98.cpp

SOURCE_C_SJIS=cFriendsShiftJis.c
SOURCE_C=cFriends.c
SOURCE_C_EXT=cFriendsExt.c
SOURCE_ERROR=cppFriendsError.cpp
INDENT_INPUT_SOURCE_H=cFriends.h
INDENT_INPUT_SOURCE_C=cFriends.c

OBJ_MAIN=cppFriendsMain.o
OBJ_FRIENDS=cppFriends.o
OBJ_SAMPLE_1=cppFriendsSample1.o
OBJ_SAMPLE_2=cppFriendsSample2.o
OBJ_SAMPLE_ASM=cppFriendsSampleAsm.o
OBJ_OPT=cppFriendsOpt.o
OBJ_EXT=cppFriendsExt.o
OBJ_THREAD=cppFriendsThread.o
OBJ_SPACE=cppFriendsSpace.o
OBJ_CLANG=cppFriendsClang.o
OBJ_CLANG_EXT=cppFriendsClangExt.o
OBJ_CPP98=cppFriends98.o

OBJ_NO_OPT=cppFriendsOpt_no_opt.o
OBJ_NO_OPT_EXT=cppFriends_no_optExt.o

OBJS=$(OBJ_MAIN) $(OBJ_FRIENDS) $(OBJ_SAMPLE_1) $(OBJ_SAMPLE_2) $(OBJ_SAMPLE_ASM)
OBJS+=$(OBJ_OPT) $(OBJ_EXT) $(OBJ_THREAD) $(OBJ_SPACE) $(OBJ_CLANG) $(OBJ_CLANG_EXT) $(OBJ_CPP98)
OBJS+=$(GTEST_OBJ) $(GMOCK_OBJ)
OBJS_NO_OPT=$(OBJ_MAIN) $(OBJ_NO_OPT) $(OBJ_NO_OPT_EXT) $(GTEST_OBJ) $(GMOCK_OBJ)
VPATH=$(dir $(GTEST_SOURCE) $(GMOCK_SOURCE))

CPP=gcc
CXX=g++
CLANGXX=clang++
LD=g++
CFLAGS=-std=gnu11 -O2 -Wall
CPPFLAGS_CPPSPEC=-std=gnu++14
CPPFLAGS_COMMON=-Wall $(GTEST_GMOCK_INCLUDE)
CPPFLAGS=$(CPPFLAGS_CPPSPEC) $(CPPFLAGS_COMMON) -O2
CPPFLAGS_NO_OPT=$(CPPFLAGS_CPPSPEC) $(CPPFLAGS_COMMON) -g -O0 -DCPPFRIENDS_NO_OPTIMIZATION
CPPFLAGS_ERROR=-std=gnu++1z -Wall $(CPPFLAGS_COMMON) -O2
LIBPATH=
LDFLAGS=
LIBS=-lboost_date_time -lboost_locale -lboost_serialization -lboost_random -lboost_regex

INDENT_OPTIONS=--line-length10000 --dont-format-comments --dont-break-function-decl-args --dont-break-procedure-type --dont-line-up-parentheses --no-space-after-parentheses

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

$(OUTPUT_ASM_C_EXT1): $(SOURCE_C_EXT)
	$(CPP) $(CFLAGS) $(CASMFLAGS) -o $@ $<

$(OUTPUT_ASM_C_EXT2): $(SOURCE_C_EXT)
	$(CPP) $(CFLAGS) $(CASMFLAGS) -DCPPFRIENDS_SIDE_EFFECT -o $@ $<

$(OUTPUT_ASM_CPP_CLANG): $(SOURCE_CLANG)
	$(CLANGXX) $(CPPFLAGS) $(CASMFLAGS) -o $@ $<

force : $(SOURCE_ERROR)
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

$(OBJ_SPACE): $(SOURCE_SPACE)
	$(CXX) $(CPPFLAGS) -o $@ -c $<

$(OBJ_CLANG): $(SOURCE_CLANG)
	$(CXX) $(CPPFLAGS) -o $@ -c $<

$(OBJ_CLANG_EXT): $(SOURCE_CLANG_EXT)
	$(CXX) $(CPPFLAGS) -o $@ -c $<

$(OBJ_CPP98): $(SOURCE_CPP98)
	$(CXX) $(CPPFLAGS_COMMON) -O2 -o $@ -c $<

$(OBJ_NO_OPT): $(SOURCE_OPT)
	$(CXX) $(CPPFLAGS_NO_OPT) -o $@ -c $<

$(OBJ_NO_OPT_EXT): $(SOURCE_EXT)
	$(CXX) $(CPPFLAGS_NO_OPT) -o $@ -c $<

$(OBJ_DIR)/%.o: %.cc
	$(CXX) $(CPPFLAGS) -o $@ -c $<

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGETS) $(EXTERNAL_TARGETS) $(OBJS) $(OBJS_NO_OPT) $(OUTPUT_ASMS) $(OUTPUT_FUNCLIST) $(OUTPUT_FUNCLIST_TEMPS) ./*.o ./*.s

show:
	$(foreach v, $(.VARIABLES), $(info $(v) = $($(v))))
