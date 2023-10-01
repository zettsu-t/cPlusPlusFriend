GTEST_GMOCK_TOP_DIR=./googletest
GTEST_TOP_DIR=$(GTEST_GMOCK_TOP_DIR)/googletest
GMOCK_TOP_DIR=$(GTEST_GMOCK_TOP_DIR)/googlemock
GTEST_GMOCK_INCLUDE=$(addprefix -isystem, $(GTEST_TOP_DIR)/include $(GTEST_TOP_DIR) $(GMOCK_TOP_DIR)/include $(GMOCK_TOP_DIR))
GTEST_SOURCE=$(GTEST_TOP_DIR)/src/gtest-all.cc
GMOCK_SOURCE=$(GMOCK_TOP_DIR)/src/gmock-all.cc
GTEST_OBJ=$(patsubst %.cc, %.o, $(notdir $(GTEST_SOURCE)))

SOURCE=thread_by_ref.cpp
OBJ=thread_by_ref.o
ASM=thread_by_ref.s

OBJS=$(OBJ)
ASMS=$(ASM)
TARGET=thread_by_ref
TARGETS=$(TARGET)

CXX=g++
CPPFLAGS=-std=gnu++17 -O2 $(GTEST_GMOCK_INCLUDE)
LD=g++
LIBPATH=
LDFLAGS=
LIBS=-lboost_program_options -pthread

.PHONY: all run clean

all: $(TARGETS)

$(TARGET): $(OBJ) $(GTEST_OBJ)
	$(LD) $(LIBPATH) -o $@ $^ $(LDFLAGS) $(LIBS)
	objdump -d -M intel --demangle $@ > $(ASM)

run: $(TARGET)
	-./$(TARGET)
	./$(TARGET) --ref ref
	./$(TARGET) --ref array

$(OBJ): $(SOURCE) Makefile
	$(CXX) $(CPPFLAGS) -c -o $@ $<

$(GTEST_OBJ): $(GTEST_SOURCE)
	$(CXX) $(CPPFLAGS) -o $@ -c $<

clean:
	rm -f $(TARGETS) $(OBJS) $(ASMS) $(GTEST_OBJ)
