ifeq ($(OS),Windows_NT)
ifneq (,$(findstring mingw,$(shell gcc -dumpmachine)))
BUILD_ON_MINGW=yes
endif
endif

GTEST_GMOCK_TOP_DIR=$(HOME)/googletest
GTEST_TOP_DIR=$(GTEST_GMOCK_TOP_DIR)/googletest
GMOCK_TOP_DIR=$(GTEST_GMOCK_TOP_DIR)/googlemock
GTEST_GMOCK_INCLUDE=$(addprefix -isystem, $(GTEST_TOP_DIR)/include $(GTEST_TOP_DIR) $(GMOCK_TOP_DIR)/include $(GMOCK_TOP_DIR))
GTEST_SOURCE=$(GTEST_TOP_DIR)/src/gtest-all.cc
GMOCK_SOURCE=$(GMOCK_TOP_DIR)/src/gmock-all.cc
GTEST_OBJ=$(patsubst %.cc, %.o, $(notdir $(GTEST_SOURCE)))

TARGET=read_csv_sample
TARGETS=$(TARGET)

SOURCE=read_csv_sample.cpp
OBJ=read_csv_sample.o
OBJS=$(OBJ) $(GTEST_OBJ)
VPATH=$(dir $(GTEST_SOURCE) $(GMOCK_SOURCE))

CPP=g++
LD=g++

CFLAGS_WALL=-O0 -g -Wall -W -Wformat=2 -Wcast-qual -Wcast-align -Wwrite-strings -Wfloat-equal -Wpointer-arith -Wno-unused-parameter
CPPFLAGS=-std=gnu++14 $(CFLAGS_WALL) $(GTEST_GMOCK_INCLUDE)
ifeq ($(BUILD_ON_MINGW),yes)
CPPFLAGS+= -isystem C:\MINGW\include
endif

.PHONY: all test clean show
.SUFFIXES: .o .cpp .cc

all: $(TARGETS) test

test: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJS)
	$(LD) $(LIBPATH) -o $@ $^ $(LDFLAGS) $(LIBS)

$(OBJ): $(SOURCE)
	$(CPP) $(CPPFLAGS) -o $@ -c $<

$(GTEST_OBJ): $(GTEST_SOURCE)
	$(CXX) $(CPPFLAGS) -o $@ -c $<

clean:
	$(RM) $(TARGETS) $(OBJS) ./*.o

show:
	$(foreach v, $(.VARIABLES), $(info $(v) = $($(v))))
