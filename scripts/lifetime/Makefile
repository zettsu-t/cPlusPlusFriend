SOURCE=lifetime.cpp

TARGET_GCC_DEBUG=lifetime_gcc_debug
TARGET_CLANG_DEBUG=lifetime_clang_debug
TARGET_GCC_RELEASE=lifetime_gcc_release
TARGET_CLANG_RELEASE=lifetime_clang_release
TARGETS=$(TARGET_GCC_DEBUG) $(TARGET_CLANG_DEBUG) $(TARGET_GCC_RELEASE) $(TARGET_CLANG_RELEASE)

ASM_GCC_DEBUG=gcc_debug.s
ASM_GCC_RELEASE=gcc_release.s
ASM_CLANG_DEBUG=clang_debug.s
ASM_CLANG_RELEASE=clang_release.s
ASMS=$(ASM_GCC_DEBUG) $(ASM_GCC_RELEASE) $(ASM_CLANG_DEBUG) $(ASM_CLANG_RELEASE)

RODATA_GCC_DEBUG=gcc_debug_ro.txt
RODATA_GCC_RELEASE=gcc_release_ro.txt
RODATA_CLANG_DEBUG=clang_debug_ro.txt
RODATA_CLANG_RELEASE=clang_release_ro.txt
RODATAS=$(RODATA_GCC_DEBUG) $(RODATA_GCC_RELEASE) $(RODATA_CLANG_DEBUG) $(RODATA_CLANG_RELEASE)

GXX=g++
CLANGXX=clang++
LD=
CPPFLAGS=-std=gnu++17 -Wall -g
ARCH=
LIBPATH=
LDFLAGS=
LIBS=

.PHONY: all run clean

all: $(TARGETS)

$(TARGET_GCC_DEBUG): $(SOURCE)
	$(GXX) $(CPPFLAGS) -O0 -o $@ $^ $(LDFLAGS) $(LIBS)
	objdump -d --demangle -M intel $@ > $(ASM_GCC_DEBUG)
	objdump -s -j .rodata $@ > $(RODATA_GCC_DEBUG)

$(TARGET_CLANG_DEBUG): $(SOURCE)
	$(CLANGXX) $(CPPFLAGS) -O0 -o $@ $^ $(LDFLAGS) $(LIBS)
	objdump -d --demangle -M intel $@ > $(ASM_CLANG_DEBUG)
	objdump -s -j .rodata $@ > $(RODATA_CLANG_DEBUG)

$(TARGET_GCC_RELEASE): $(SOURCE)
	$(GXX) $(CPPFLAGS) -O2 -o $@ $^ $(LDFLAGS) $(LIBS)
	objdump -d --demangle -M intel $@ > $(ASM_GCC_RELEASE)
	objdump -s -j .rodata $@ > $(RODATA_GCC_RELEASE)

$(TARGET_CLANG_RELEASE): $(SOURCE)
	$(CLANGXX) $(CPPFLAGS) -O2 -o $@ $^ $(LDFLAGS) $(LIBS)
	objdump -d --demangle -M intel $@ > $(ASM_CLANG_RELEASE)
	objdump -s -j .rodata $@ > $(RODATA_CLANG_RELEASE)

run:
	-./$(TARGET_GCC_DEBUG)
	-./$(TARGET_CLANG_DEBUG)
	-./$(TARGET_GCC_RELEASE)
	-./$(TARGET_CLANG_RELEASE)

clean:
	rm -f $(TARGETS) $(ASMS) $(RODATAS)
