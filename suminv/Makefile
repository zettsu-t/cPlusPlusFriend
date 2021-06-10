TARGET=suminv
TARGETS=$(TARGET)

SOURCE_CPP=suminv.cpp
SOURCE_ASM=suminv_asm.s
OBJ_CPP=suminv.o
OBJ_ASM=suminv_asm.o
OBJS=$(OBJ_CPP) $(OBJ_ASM)

CXX=g++
AS=as
LD=g++
CPPFLAGS=-std=gnu++17 -O2 -Wall -mavx2 -masm=intel -Wall -W -Wconversion -Wformat=2 -Wcast-qual -Wcast-align -Wwrite-strings -Wconversion -Wfloat-equal -Wpointer-arith -Wno-unused-parameter
LIBPATH=
LDFLAGS=
LIBS=

TIME=time
SMALL_BORDER=22
LARGE_BORDER=30

.PHONY: all clean rebuild test shortrun longrun

all: $(TARGETS)

$(TARGET) : $(OBJS)
	$(LD) $(LIBPATH) -o $@ $^ $(LDFLAGS) $(LIBS)

$(OBJ_CPP) : $(SOURCE_CPP)
	$(CXX) $(CPPFLAGS) -o $@ -c $<

$(OBJ_ASM) : $(SOURCE_ASM)
	$(AS) -o $@ $<

test: $(TARGET)
	./$(TARGET)

shortrun: $(TARGET)
	$(TIME) ./$(TARGET) $(SMALL_BORDER)
	$(TIME) ./$(TARGET) $(SMALL_BORDER)
	$(TIME) ./$(TARGET) $(SMALL_BORDER)

longrun: $(TARGET)
	$(TIME) ./$(TARGET) $(LARGE_BORDER)
	$(TIME) ./$(TARGET) $(LARGE_BORDER)
	$(TIME) ./$(TARGET) $(LARGE_BORDER)

clean:
	$(RM) $(TARGETS) $(OBJS)

rebuild : clean all
