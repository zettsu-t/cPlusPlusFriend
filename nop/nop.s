.intel_syntax noprefix
.file   "nop.s"

## 0x48 REX.W 0100b (fixed) and 1000b (64 Bit Operand Size)
## 0x87 XCHG
## 0xc0
##   2-bit ModR/M.mod (11b : direct register access)
##   3-bit a destination register operand
##   3-bit a source register operand

.text
    nop                # 90
    xchg     rax, rax  # 90
    .byte    0x48      # 48 87 c0
    .byte    0x87
    .byte    0xc0
    xchg     rcx, rcx  # 48 87 c9
    xchg     rdx, rdx  # 48 87 d2
    xchg     rbx, rbx  # 48 87 db
