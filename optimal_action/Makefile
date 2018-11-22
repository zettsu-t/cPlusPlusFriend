TARGET=optimal_action
SOURCE=optimal_action.cpp

CXX=g++
CPPFLAGS=-std=gnu++14 -O2 -Wall
LIBPATH=
LDFLAGS=

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SOURCE) Makefile
	$(CXX) $(CPPFLAGS) $(LIBPATH) -o $@ $(SOURCE) $(LDFLAGS) $(LIBS)
	./$(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
