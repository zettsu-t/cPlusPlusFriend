SOURCE=recursion.cpp
OBJ=recursion.o
ASM=recursion.s
OBJS=$(OBJ)
TARGET=recursion
TARGETS=$(TARGET)

CXX=g++
CPPFLAGS=-g -O3
LD=g++
LIBPATH=
LDFLAGS=
LIBS=

.PHONY: all run clean

all: $(TARGETS)

$(TARGET): $(OBJ)
	$(LD) $(LIBPATH) -o $@ $^ $(LDFLAGS) $(LIBS)
	objdump -d -M intel --demangle $@ > $(ASM)
	./$@

$(OBJ): $(SOURCE) Makefile
	$(CXX) $(CPPFLAGS) -c -o $@ $<

run:
	./$(TARGET)

clean:
	rm -f $(TARGETS) $(OBJS)
