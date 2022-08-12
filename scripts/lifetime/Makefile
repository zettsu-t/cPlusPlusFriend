SOURCE=lifetime.cpp
TARGET=lifetime

CXX=clang++
LD=
CPPFLAGS=-std=gnu++17 -Wall -g -O0
ARCH=
LIBPATH=
LDFLAGS=
LIBS=

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CPPFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)
	./$(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)
