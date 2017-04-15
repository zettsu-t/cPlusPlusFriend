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
TARGET_C=cFriends
TARGETS=$(TARGET) $(TARGET_C)

SOURCE=cppFriends.cpp
SOURCE_C=cFriends.c
SOURCE_ERROR=cppFriendsError.cpp

OBJ=cppFriends.o
OBJS=$(OBJ) $(GTEST_OBJ) $(GMOCK_OBJ)
VPATH=$(dir $(GTEST_SOURCE) $(GMOCK_SOURCE))

CPP=gcc
CXX=g++
LD=g++
CFLAGS=-std=c99 -O2 -Wall -Werror
CPPFLAGS=-std=gnu++14 -O2 -Wall -Werror $(GTEST_GMOCK_INCLUDE)
LIBPATH=
LDFLAGS=
LIBS=-lboost_serialization

.PHONY: all test clean force
.SUFFIXES: .o .cpp .cc

all: $(TARGETS) force

$(TARGET): $(OBJS)
	$(LD) $(LIBPATH) -o $@ $^ $(LDFLAGS) $(LIBS)
	./$@

$(TARGET_C): $(SOURCE_C)
	$(CPP) $(CFLAGS) -o $@ $<
	./$@
	-$(CXX) $(CPPFLAGS) -c $<

force : $(SOURCE_ERROR)
	-$(CXX) $(CPPFLAGS) -c $<

$(OBJ): $(SOURCE)
	$(CXX) $(CPPFLAGS) -o $@ -c $<

$(OBJ_DIR)/%.o: %.cc
	$(CXX) $(CPPFLAGS) -o $@ -c $<

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGETS) $(OBJS) ./*.o
