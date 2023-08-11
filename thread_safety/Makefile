GTEST_GMOCK_TOP_DIR=./googletest
GTEST_TOP_DIR=$(GTEST_GMOCK_TOP_DIR)/googletest
GMOCK_TOP_DIR=$(GTEST_GMOCK_TOP_DIR)/googlemock
GTEST_GMOCK_INCLUDE=$(addprefix -isystem, $(GTEST_TOP_DIR)/include $(GTEST_TOP_DIR) $(GMOCK_TOP_DIR)/include $(GMOCK_TOP_DIR))
GTEST_SOURCE=$(GTEST_TOP_DIR)/src/gtest-all.cc
GMOCK_SOURCE=$(GMOCK_TOP_DIR)/src/gmock-all.cc
GTEST_OBJ=$(patsubst %.cc, %.o, $(notdir $(GTEST_SOURCE)))

SOURCE=thread_safety.cpp
OBJ=thread_safety.o
ASM=thread_safety.s
OBJS=$(OBJ)
TARGET=thread_safety
TARGETS=$(TARGET)

CXX=g++
CPPFLAGS=-std=gnu++17 -g -O0 $(GTEST_GMOCK_INCLUDE)
LD=g++
LIBPATH=
LDFLAGS=
LIBS=-lboost_program_options -pthread

.PHONY: all run clean

all: $(TARGETS)

$(TARGET): $(OBJ) $(GTEST_OBJ)
	$(LD) $(LIBPATH) -o $@ $^ $(LDFLAGS) $(LIBS)
	objdump -d -M intel --demangle $@ > $(ASM)

$(OBJ): $(SOURCE) Makefile
	$(CXX) $(CPPFLAGS) -c -o $@ $<

$(GTEST_OBJ): $(GTEST_SOURCE)
	$(CXX) $(CPPFLAGS) -o $@ -c $<

run:
	./$(TARGET)

clean:
	rm -f $(TARGETS) $(OBJS)
