SOURCE_AS=nop.s
SOURCE_NASM=nop.asm
OBJ_AS=nop_as.o
OBJ_NASM=nop_nasm.o
TARGET_AS=nop_as.txt
TARGET_NASM=nop_nasm.txt

OBJS=$(OBJ_AS) $(OBJ_NASM)
TARGETS=$(TARGET_AS) $(TARGET_NASM)

AS=as
NASM=nasm
DISASM=objdump

AS_FLAGS=
NASM_FLAGS=-f elf64
DISASM_FLAGS=-d -M intel

.PHONY: all run clean

all: $(TARGETS)

$(TARGET_AS): $(OBJ_AS)
	$(DISASM) $(DISASM_FLAGS) $< | tee $@

$(TARGET_NASM): $(OBJ_NASM)
	$(DISASM) $(DISASM_FLAGS) $< | tee $@

$(OBJ_AS): $(SOURCE_AS)
	$(AS) $(AS_FLAGS) $< -o $@

$(OBJ_NASM): $(SOURCE_NASM)
	$(NASM) $(NASM_FLAGS) $< -o $@

clean:
	rm -f $(TARGETS) $(OBJS)
