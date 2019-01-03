ifeq ($(OS),Windows_NT)
ifneq (,$(findstring mingw,$(shell gcc -dumpmachine)))
BUILD_ON_MINGW=yes
endif
endif

TARGET=negative_binomial_cpp
TARGETS=$(TARGET)
SOURCE=negative_binomial_cpp.cpp
OBJ=negative_binomial_cpp.o
OBJS=$(OBJ)

CPP=g++
LD=g++
CPPFLAGS=-std=gnu++14 -Wall -W -Wformat=2 -Wcast-qual -Wcast-align -Wwrite-strings -Wfloat-equal -Wpointer-arith -Wno-unused-parameter
ifeq ($(BUILD_ON_MINGW),yes)
CPPFLAGS+= -isystem C:\MINGW\include
endif
LDFLAGS=
LIBS=-lboost_random

RSCRIPT=Rscript
RSOURCE=negative_binomial_cpp.R

.PHONY: all run clean show
.SUFFIXES: .o .cpp

all: $(TARGETS) run

run: $(TARGET)
	./$(TARGET)
	$(RSCRIPT) $(RSOURCE)

$(TARGET): $(OBJS)
	$(LD) $(LIBPATH) -o $@ $^ $(LDFLAGS) $(LIBS)

$(OBJ): $(SOURCE)
	$(CPP) $(CPPFLAGS) -o $@ -c $<

clean:
	$(RM) $(TARGETS) $(OBJS) ./*.o

show:
	$(foreach v, $(.VARIABLES), $(info $(v) = $($(v))))
