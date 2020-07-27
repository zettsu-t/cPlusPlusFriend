SOURCE=count_patterns.cpp
OBJ=count_patterns.o
OBJS=$(OBJ)
TARGET=count_patterns
TARGETS=$(TARGET)

CXX=g++
OPT=-O3
LD=g++
CPPFLAGS=-std=gnu++17 $(OPT) -Wall
LIBPATH=
LDFLAGS=
LIBS=

ifeq (clang++,$(CXX))
CLANG_TARGET=
endif

.PHONY: all run clean

all: $(TARGETS)

$(TARGET): $(OBJ)
	$(LD) $(LIBPATH) -o $@ $^ $(LDFLAGS) $(LIBS)
	./$@

$(OBJ): $(SOURCE) Makefile
	$(CXX) $(CLANG_TARGET) $(CPPFLAGS) -o $@ -c $<

run:
	./$(TARGET)

clean:
	rm -f $(TARGETS) $(OBJS)
