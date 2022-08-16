SOURCE=recursion.cpp
OBJ=recursion.o
ASM=recursion.s
OBJS=$(OBJ)
TARGET=recursion
TARGETS=$(TARGET)

AS=as
CXX=g++
CLANGXX=clang++
CPPFLAGS=-g -O3
LD=g++
LIBPATH=
LDFLAGS=
LIBS=

.PHONY: all run clean

all: $(TARGETS)

$(TARGET): $(OBJ)
	$(LD) $(LIBPATH) -o $@ $^ $(LDFLAGS) $(LIBS) $(CLANGXX_LDFLAGS) $(CLANGXX_LIBS)
	objdump -d -M intel --demangle $@ > $(ASM)
	./$@

$(OBJ): $(SOURCE) Makefile
	$(CXX) $(CPPFLAGS) -c -o $@ $<

run:
	./$(TARGET)

clean:
	rm -f $(TARGETS) $(OBJS)
