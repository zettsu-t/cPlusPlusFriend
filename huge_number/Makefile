TARGET_GCC=ackermann_function_gcc
TARGET_CLANG=ackermann_function_clang
TARGETS=$(TARGET_GCC) $(TARGET_CLANG)
SOURCE=ackermann_function.cpp
CPPFLAGS=-std=c++14 -Wall

.PHONY: all run clean show
.SUFFIXES: .o .cpp

all: $(TARGETS) run

run: $(TARGETS)
	./$(TARGET_GCC)
	./$(TARGET_CLANG)

$(TARGET_GCC): $(SOURCE)
	-g++ -O2 $(CPPFLAGS) -o $@ $<

$(TARGET_CLANG): $(SOURCE)
	-clang++ -O2 $(CPPFLAGS) -o $@ $<

clean:
	$(RM) $(TARGETS) ./*.o

show:
	$(foreach v, $(.VARIABLES), $(info $(v) = $($(v))))
