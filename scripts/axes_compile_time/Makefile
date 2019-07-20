TARGET=axes_compile_time
SOURCE=axes_compile_time.cpp
NUMBERS=axes_compile_time.inc
NUMBERS_PYTHON=axes_compile_time_py.inc
CODEGEN_R=axes_compile_time.R
CODEGEN_PYTHON=axes_compile_time.py
NUM_SIZE=100000

CXX=g++
CPPFLAGS=-std=gnu++14 -O2 -Wall -isystem C:\MinGW\include
LIBPATH=
LDFLAGS=
LIBS=

RSCRIPT=Rscript
PYTHON=C:\bin\Anaconda\python
TIME=C:\cygwin64\bin\time

.PHONY: all run codegen clean

all: $(TARGET)

$(TARGET): $(SOURCE) $(NUMBERS) Makefile
	$(CXX) $(CPPFLAGS) $(LIBPATH) -o $@ $(SOURCE) $(LDFLAGS) $(LIBS)
	./$(TARGET)

$(NUMBERS): $(CODEGEN_R) Makefile
	$(RSCRIPT) $(CODEGEN_R)

run:
	./$(TARGET)

codegen:
	$(TIME) $(RSCRIPT) $(CODEGEN_R) $(NUM_SIZE)
	head -1 $(NUMBERS) | tr ',' '\n' | wc -l
	$(TIME) $(PYTHON) $(CODEGEN_PYTHON) $(NUM_SIZE)
	head -1 $(NUMBERS_PYTHON) | tr ',' '\n' | wc -l

clean:
	rm -f $(TARGET) $(NUMBERS) $(NUMBERS_PYTHON)
