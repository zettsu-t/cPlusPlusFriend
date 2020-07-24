SOURCE=river.cpp
OBJ=river.o
TARGET=river
OBJS=$(OBJ)

CXX=g++
OPT=-O2

LD=g++
CPPFLAGS=-std=gnu++17 $(OPT) -Wall $(GTEST_GMOCK_INCLUDE)
ARCH=
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

$(OBJ): $(SOURCE) Makefile
	$(CXX) $(CLANG_TARGET) $(CPPFLAGS) $(ARCH) -o $@ -c $<

clean:
	rm -f $(TARGET) $(OBJS)
