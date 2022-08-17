SOURCE=cpp_sample.cpp
OBJ=cpp_sample.o
ASM=cpp_sample.s
OBJS=$(OBJ)
TARGET=cpp_sample
TARGETS=$(TARGET)

CXX=g++
CPPFLAGS=-std=gnu++2a -g -O3
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
