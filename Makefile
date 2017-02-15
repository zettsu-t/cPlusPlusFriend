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
SOURCE=cppFriends.cpp
OBJS=cppFriends.o $(GTEST_OBJ) $(GMOCK_OBJ)
VPATH=$(dir $(GTEST_SOURCE) $(GMOCK_SOURCE))

CXX=g++
LD=g++
CPPFLAGS=-std=gnu++14 -g -O -Wall $(GTEST_GMOCK_INCLUDE)
LIBPATH=
LDFLAGS=
LIBS=-lboost_serialization

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) $(LIBPATH) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)
	./$(TARGET)

$(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CPPFLAGS) -o $@ -c $<

$(OBJ_DIR)/%.o: %.cc
	$(CXX) $(CPPFLAGS) -o $@ -c $<

clean:
	rm -f $(TARGET) $(OBJS) ./*.o
