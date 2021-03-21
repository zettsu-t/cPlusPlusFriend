SOURCE=element_chain.cpp
OBJ=element_chain.o
OBJS=$(OBJ)
TARGET=element_chain
TARGETS=$(TARGET)

CXX=g++
OPT=-O3
LD=g++
CPPFLAGS=-std=gnu++17 $(OPT) -Wall -W -Wconversion -Wformat=2 -Wcast-qual -Wcast-align -Wwrite-strings -Wconversion -Wfloat-equal -Wpointer-arith -Wno-unused-parameter
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

$(OBJ): $(SOURCE) Makefile
	$(CXX) $(CLANG_TARGET) $(CPPFLAGS) -o $@ -c $<

run:
	./$(TARGET)

clean:
	rm -f $(TARGETS) $(OBJS)
