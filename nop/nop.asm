bits 64
section .text
    nop                ; 90
    xchg     rax, rax  ; 48 90
    db       0x48      ; 48 87 c0
    db       0x87
    db       0xc0
    xchg     rcx, rcx  ; 48 87 c9
    xchg     rdx, rdx  ; 48 87 d2
    xchg     rbx, rbx  ; 48 87 db
