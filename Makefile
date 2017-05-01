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
TARGETS=$(TARGET) $(TARGET_NO_OPT) $(TARGET_C_SJIS) $(TARGET_C)
OUTPUT_ASM87_C=cFriends87.s
OUTPUT_ASM87_STORE_C=cFriends87-store.s
OUTPUT_ASM64_C=cFriends64.s
OUTPUT_ASMS=$(OUTPUT_ASM87_C) $(OUTPUT_ASM87_STORE_C) $(OUTPUT_ASM64_C)

CASMFLAGS=-S -masm=intel
CASMFLAGS_87=-mfpmath=387 -mno-sse
CASMFLAGS_87_STORE=-mfpmath=387 -mno-sse -ffloat-store
CASMFLAGS_64=

SOURCE_MAIN=cppFriends.cpp
SOURCE_EXT=cppFriendsExt.cpp
SOURCE_THREAD=cppFriendsThread.cpp
SOURCE_C_SJIS=cFriendsShiftJis.c
SOURCE_C=cFriends.c
SOURCE_ERROR=cppFriendsError.cpp

OBJ_MAIN=cppFriends.o
OBJ_THREAD=cppFriendsThread.o
OBJ_EXT=cppFriendsExt.o
OBJ_NO_OPT_MAIN=cppFriends_no_opt.o
OBJ_NO_OPT_EXT=cppFriends_no_optExt.o
OBJS=$(OBJ_MAIN) $(OBJ_THREAD) $(OBJ_EXT) $(GTEST_OBJ) $(GMOCK_OBJ)
OBJS_NO_OPT=$(OBJ_NO_OPT_MAIN) $(OBJ_NO_OPT_EXT) $(GTEST_OBJ) $(GMOCK_OBJ)
VPATH=$(dir $(GTEST_SOURCE) $(GMOCK_SOURCE))

CPP=gcc
CXX=g++
LD=g++
CFLAGS=-std=gnu11 -O2 -Wall
CPPFLAGS_CPPSPEC=-std=gnu++14
CPPFLAGS_COMMON=-Wall $(GTEST_GMOCK_INCLUDE)
CPPFLAGS=$(CPPFLAGS_CPPSPEC) $(CPPFLAGS_COMMON) -O2
CPPFLAGS_NO_OPT=$(CPPFLAGS_CPPSPEC) $(CPPFLAGS_COMMON) -O0 -DCPPFRIENDS_NO_OPTIMIZATION
CPPFLAGS_ERROR=-std=gnu++1z -Wall $(CPPFLAGS_COMMON) -O2
LIBPATH=
LDFLAGS=
LIBS=-lboost_serialization -lboost_regex

.PHONY: all test clean cprog force
.SUFFIXES: .o .cpp .cc

all: $(TARGETS) force

$(TARGET): $(OBJS)
	$(LD) $(LIBPATH) -o $@ $^ $(LDFLAGS) $(LIBS)
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
	$(CPP) $(CFLAGS) $(CASMFLAGS) $(CASMFLAGS_87) -o $(OUTPUT_ASM87_C)  $<
	$(CPP) $(CFLAGS) $(CASMFLAGS) $(CASMFLAGS_87_STORE) -o $(OUTPUT_ASM87_STORE_C)  $<
	$(CPP) $(CFLAGS) $(CASMFLAGS) $(CASMFLAGS_64) -o $(OUTPUT_ASM64_C)  $<
	$(CPP) $(CFLAGS) $(CASMFLAGS_87) -o $@ $<
	@echo -e $(ECHO_START)Using x87 and compile with $(CASMFLAGS_87) $(ECHO_END)
	@./$@
	$(CPP) $(CFLAGS) $(CASMFLAGS_87_STORE) -o $@ $<
	@echo -e $(ECHO_START)Using x87 and compile with $(CASMFLAGS_87_STORE) $(ECHO_END)
	@./$@
	$(CPP) $(CFLAGS) $(CASMFLAGS_64) -o $@ $<
	@echo -e $(ECHO_START)Using x64 and SSE $(ECHO_END)
	@./$@
	-$(CXX) $(CPPFLAGS) -c $<

force : $(SOURCE_ERROR)
	-$(CXX) $(CPPFLAGS_ERROR) -c $<

$(OBJ_MAIN): $(SOURCE_MAIN)
	$(CXX) $(CPPFLAGS) -o $@ -c $<

$(OBJ_THREAD): $(SOURCE_THREAD)
	$(CXX) $(CPPFLAGS) -o $@ -c $<

$(OBJ_EXT): $(SOURCE_EXT)
	$(CXX) $(CPPFLAGS) -o $@ -c $<

$(OBJ_NO_OPT_MAIN): $(SOURCE_MAIN)
	$(CXX) $(CPPFLAGS_NO_OPT) -o $@ -c $<

$(OBJ_NO_OPT_EXT): $(SOURCE_EXT)
	$(CXX) $(CPPFLAGS_NO_OPT) -o $@ -c $<

$(OBJ_DIR)/%.o: %.cc
	$(CXX) $(CPPFLAGS) -o $@ -c $<

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGETS) $(OBJS) $(OBJS_NO_OPT) $(OUTPUT_ASMS) ./*.o ./*.s
