GTEST_GMOCK_TOP_DIR=./googletest
GTEST_TOP_DIR=$(GTEST_GMOCK_TOP_DIR)/googletest
GMOCK_TOP_DIR=$(GTEST_GMOCK_TOP_DIR)/googlemock
GTEST_GMOCK_INCLUDE=$(addprefix -isystem, $(GTEST_TOP_DIR)/include $(GTEST_TOP_DIR) $(GMOCK_TOP_DIR)/include $(GMOCK_TOP_DIR))
GTEST_SOURCE=$(GTEST_TOP_DIR)/src/gtest-all.cc
GMOCK_SOURCE=$(GMOCK_TOP_DIR)/src/gmock-all.cc
GTEST_OBJ=$(patsubst %.cc, %.o, $(notdir $(GTEST_SOURCE)))

SOURCE_THREAD_SAFETY=thread_safety.cpp
OBJ_THREAD_SAFETY=thread_safety.o
ASM_THREAD_SAFETY=thread_safety.s

SOURCE_STD_EXECUTION=std_execution.cpp
OBJ_STD_EXECUTION=std_execution.o
ASM_STD_EXECUTION=std_execution.s

OBJS=$(OBJ_THREAD_SAFETY) $(OBJ_STD_EXECUTION)
ASMS=$(ASM_THREAD_SAFETY) $(ASM_STD_EXECUTION)
TARGET_THREAD_SAFETY=thread_safety
TARGET_STD_EXECUTION=std_execution
TARGETS=$(TARGET_THREAD_SAFETY) $(TARGET_STD_EXECUTION)

CXX=g++
CPPFLAGS=-std=gnu++17 -O2 $(GTEST_GMOCK_INCLUDE)
LD=g++
LIBPATH=
LDFLAGS=
LIBS=-lboost_program_options -ltbb -pthread

.PHONY: all run clean

all: $(TARGETS)

$(TARGET_THREAD_SAFETY): $(OBJ_THREAD_SAFETY) $(GTEST_OBJ)
	$(LD) $(LIBPATH) -o $@ $^ $(LDFLAGS) $(LIBS)
	objdump -d -M intel --demangle $@ > $(ASM_THREAD_SAFETY)

$(TARGET_STD_EXECUTION): $(OBJ_STD_EXECUTION) $(GTEST_OBJ)
	$(LD) $(LIBPATH) -o $@ $^ $(LDFLAGS) $(LIBS)
	objdump -d -M intel --demangle $@ > $(ASM_STD_EXECUTION)

$(OBJ_THREAD_SAFETY): $(SOURCE_THREAD_SAFETY) Makefile
	$(CXX) $(CPPFLAGS) -c -o $@ $<

$(OBJ_STD_EXECUTION): $(SOURCE_STD_EXECUTION) Makefile
	$(CXX) $(CPPFLAGS) -c -o $@ $<

$(GTEST_OBJ): $(GTEST_SOURCE)
	$(CXX) $(CPPFLAGS) -o $@ -c $<

run: $(TARGETS)
	./$(TARGET_THREAD_SAFETY)
	./$(TARGET_STD_EXECUTION)

clean:
	rm -f $(TARGETS) $(OBJS) $(ASMS) $(GTEST_OBJ)
