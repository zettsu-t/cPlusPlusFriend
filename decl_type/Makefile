GTEST_GMOCK_TOP_DIR=$(HOME)/googletest
GTEST_TOP_DIR=$(GTEST_GMOCK_TOP_DIR)/googletest
GTEST_GMOCK_INCLUDE=$(addprefix -isystem, $(GTEST_TOP_DIR)/include $(GTEST_TOP_DIR) $(GMOCK_TOP_DIR)/include $(GMOCK_TOP_DIR))
GTEST_SOURCE=$(GTEST_TOP_DIR)/src/gtest-all.cc
GTEST_OBJ=$(patsubst %.cc, %.o, $(notdir $(GTEST_SOURCE)))

SOURCE=decl_type.cpp
OBJ=decl_type.o
TARGET=decl_type

OBJS=$(OBJ) $(GTEST_OBJ)

CXX=g++
##CXX=clang++
OPT=-O0 -g
##OPT=-O2

LD=g++
CPPFLAGS=-std=gnu++17 $(OPT) -Wall $(GTEST_GMOCK_INCLUDE)
LIBPATH=
LDFLAGS=
LIBS=

ifeq (,$(findstring cygwin,$(shell gcc -dumpmachine)))
CPPFLAGS+=-isystem C:\MinGW\include
ifeq (clang++,$(CXX))
##CLANG_TARGET=-target x86_64-pc-windows-gnu
endif
endif

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) $(LIBPATH) -o $@ $^ $(LDFLAGS) $(LIBS)
	./$(TARGET)

run : $(TARGET)
	./$(TARGET)

$(OBJ): $(SOURCE)
	$(CXX) $(CLANG_TARGET) $(CPPFLAGS) -o $@ -c $<

$(GTEST_OBJ): $(GTEST_SOURCE)
	$(CXX) $(CLANG_TARGET) $(CPPFLAGS) -o $@ -c $<

clean:
	rm -f $(TARGET) $(OBJS)
