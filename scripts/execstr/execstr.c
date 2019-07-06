#include "bintext.txt"

int main(int argc, char* argv[]) {
    asm volatile (
        ".byte " BIN_TEXT "\n\t"
        :::"rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
    return 0;
}

/*
Local Variables:
mode: c
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
