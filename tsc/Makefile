TARGET=tsc
SOURCE_CPP=tsc.cpp

ifneq ($(OS),Windows_NT)
SOURCE_ASM=tsc_linux.asm
else
SOURCE_ASM=tsc_win.asm
endif

OBJ_CPP=tsc_cpp.o
OBJ_ASM=tsc_asm.o

OBJS=$(OBJ_CPP) $(OBJ_ASM)

CXX=g++
CPPFLAGS=-std=gnu++17 -O0 -g -Wall -mavx2 -masm=intel
LIBPATH=
LDFLAGS=
LIBS=

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LIBPATH) -o $@ $(LDFLAGS) $^ $(LIBS)

$(OBJ_CPP): $(SOURCE_CPP) Makefile
	$(CXX) -c $(CPPFLAGS) $< -o $@

$(OBJ_ASM): $(SOURCE_ASM) Makefile
	nasm -f elf64 -o $@ $<

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)
