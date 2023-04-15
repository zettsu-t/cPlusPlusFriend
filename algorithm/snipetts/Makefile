GTEST_GMOCK_TOP_DIR=./googletest
GTEST_TOP_DIR=$(GTEST_GMOCK_TOP_DIR)/googletest
GMOCK_TOP_DIR=$(GTEST_GMOCK_TOP_DIR)/googlemock
GTEST_GMOCK_INCLUDE=$(addprefix -isystem, $(GTEST_TOP_DIR)/include $(GTEST_TOP_DIR) $(GMOCK_TOP_DIR)/include $(GMOCK_TOP_DIR))
GTEST_SOURCE=$(GTEST_TOP_DIR)/src/gtest-all.cc
GMOCK_SOURCE=$(GMOCK_TOP_DIR)/src/gmock-all.cc
GTEST_OBJ=$(patsubst %.cc, %.o, $(notdir $(GTEST_SOURCE)))

SOURCES=$(wildcard ./*.cpp)
TARGETS=$(notdir $(basename $(SOURCES)))

CXX=g++
LD=g++
CPPFLAGS=-std=gnu++17 -Wall -Wextra -O0 -g $(GTEST_GMOCK_INCLUDE)
LIBPATH=
LDFLAGS=
LIBS=-pthread

.PHONY: all clean run show
.SUFFIXES: .cpp

all: $(TARGETS)

$(TARGETS) : % : %.cpp $(GTEST_OBJ)
	$(LD) $(LIBPATH) $(CPPFLAGS) -o $@ $< $(GTEST_OBJ) $(LDFLAGS) $(LIBS)

$(GTEST_OBJ): $(GTEST_SOURCE)
	$(CXX) $(CPPFLAGS) -o $@ -c $<

run: $(TARGETS)
	for f in $^ ; do echo ./$$f && ./$$f ; done

clean:
	rm -f $(TARGETS)

show:
	$(foreach v, $(.VARIABLES), $(info $(v) = $($(v))))
