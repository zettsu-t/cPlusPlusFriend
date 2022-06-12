SOURCE=ambiguous.cpp
TARGET=ambiguous

CXX=g++
LD=g++
CPPFLAGS=-std=gnu++17 -Wall
ARCH=
LIBPATH=
LDFLAGS=
LIBS=

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(LD) $(CPPFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)
