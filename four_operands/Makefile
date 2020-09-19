TARGET=four_operands
SOURCE=four_operands.cpp

CXX=g++
CPPFLAGS=-std=gnu++17 -O2 -Wall -mavx2 -masm=intel
LIBPATH=
LDFLAGS=

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SOURCE) Makefile
	$(CXX) $(CPPFLAGS) $(LIBPATH) -o $@ $(SOURCE) $(LDFLAGS) $(LIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
