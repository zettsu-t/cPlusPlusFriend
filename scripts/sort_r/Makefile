ifeq ($(OS),Windows_NT)
ifneq (,$(findstring mingw,$(shell gcc -dumpmachine)))
BUILD_ON_MINGW=yes
endif
endif

TARGET=sort_fast
TARGETS=$(TARGET)
SOURCE=sort_fast.cpp
OBJ=sort_fast.o
OBJS=$(OBJ)

CPP=g++
LD=g++
CPPFLAGS=-std=gnu++14 -Wall -W -Wformat=2 -Wcast-qual -Wcast-align -Wwrite-strings -Wfloat-equal -Wpointer-arith -Wno-unused-parameter -DTESTING_FROM_MAIN
ifeq ($(BUILD_ON_MINGW),yes)
CPPFLAGS+= -isystem C:\MINGW\include
endif
LDFLAGS=
LIBS=

RSCRIPT=Rscript
RSOURCE=sort_fast.R

.PHONY: all run clean show
.SUFFIXES: .o .cpp

all: $(TARGETS) run

run: $(TARGET)
	./$(TARGET)
#	$(RSCRIPT) $(RSOURCE)

$(TARGET): $(OBJS)
	$(LD) $(LIBPATH) -o $@ $^ $(LDFLAGS) $(LIBS)

$(OBJ): $(SOURCE)
	$(CPP) $(CPPFLAGS) -o $@ -c $<

clean:
	$(RM) $(TARGETS) $(OBJS) ./*.o

show:
	$(foreach v, $(.VARIABLES), $(info $(v) = $($(v))))
