.intel_syntax noprefix
.file "suminv_asm.s"

.set  gSizeOfDivisors, rax

.set  yZeros,      ymm0
.set  yOnes,       ymm1
.set  yFours,      ymm2
.set  yBorders,    ymm3
.set  yWork,       ymm4
.set  yWork2,      ymm5
.set  yQuots,      ymm6
.set  ySums,       ymm7
.set  yLess,       yWork2

.set  yDivisors0,  ymm8
.set  yDivisors1,  ymm9
.set  yDivisors2,  ymm10
.set  yDivisors3,  ymm11
.set  yDivisors4,  ymm12
.set  yDivisors5,  ymm13
.set  yDivisors6,  ymm14
.set  yDivisors7,  ymm15

.data
.align 32
data_zeros:    .long  0, 0, 0, 0
data_ones:     .long  1, 1, 1, 1
data_fours:    .long  4, 4, 4, 4
data_divisors: .long  4, 3, 2, 1

.align 32
ymm_register_set:
    .quad 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
    .quad 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
    .quad 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
    .quad 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
    .quad 0

.text
.global sumUpto

.macro MacroSumDiv quots, sums, work, work2
    vpermq     \work, \quots, 0xff
    vaddpd     \sums, \sums, \work

    vpermq     \work,  \quots, 0xaa
    vshufpd    \work2, \work, yZeros, 0
    vperm2f128 \work,  \work, \work2, 0x20
    vaddpd     \sums,  \sums, \work

    vpermq     \work, \quots, 0x55
    vperm2f128 \work, \work, yZeros, 0x20
    vaddpd     \sums, \sums, \work

    vpermq     \work, \quots, 0
    vshufpd    \work, \work, yZeros, 0
    vperm2f128 \work, \work, yZeros, 0x20
    vaddpd     \sums, \sums, \work
.endm

.macro MacroSumDivExpand divisors, divisorsNext, sumAddr, addDivisors
    vdivpd      yQuots, yOnes, \divisors
.if (\addDivisors != 0)
    vaddpd      \divisorsNext, \divisors, yFours
.endif
    MacroSumDiv yQuots, ySums, yWork, yWork2
    vmovdqa     [rdi + \sumAddr], ySums
    vpermq      ySums, ySums, 0
.endm

sumUpto:
    vmovdqa   ymmword ptr [rsi + 32],  ymm0
    vmovdqa   ymmword ptr [rsi + 64],  ymm1
    vmovdqa   ymmword ptr [rsi + 96],  ymm2
    vmovdqa   ymmword ptr [rsi + 128], ymm3
    vmovdqa   ymmword ptr [rsi + 160], ymm4
    vmovdqa   ymmword ptr [rsi + 192], ymm5
    vmovdqa   ymmword ptr [rsi + 224], ymm6
    vmovdqa   ymmword ptr [rsi + 256], ymm7
    vmovdqa   ymmword ptr [rsi + 288], ymm8
    vmovdqa   ymmword ptr [rsi + 320], ymm9
    vmovdqa   ymmword ptr [rsi + 352], ymm10
    vmovdqa   ymmword ptr [rsi + 384], ymm11
    vmovdqa   ymmword ptr [rsi + 416], ymm12
    vmovdqa   ymmword ptr [rsi + 448], ymm13
    vmovdqa   ymmword ptr [rsi + 480], ymm14
    vmovdqa   ymmword ptr [rsi + 512], ymm15

    vcvtdq2pd yBorders,   [rsi]
    vcvtdq2pd yZeros,     [rip + data_zeros]
    vcvtdq2pd yOnes,      [rip + data_ones]
    vcvtdq2pd yFours,     [rip + data_fours]
    vcvtdq2pd yDivisors0, [rip + data_divisors]
    vcvtdq2pd ySums,      [rip + data_zeros]

    cmp gSizeOfDivisors, 1
    je  divNext1

    cmp gSizeOfDivisors, 2
    je  divNext2

    cmp gSizeOfDivisors, 3
    je  divNext3

    cmp gSizeOfDivisors, 4
    je  divNext4

    cmp gSizeOfDivisors, 5
    je  divNext5

    cmp gSizeOfDivisors, 6
    je  divNext6

    cmp gSizeOfDivisors, 7
    je  divNext7

    cmp gSizeOfDivisors, 8
    je  divNext8

divNext0:
    vaddpd   yDivisors0, yOnes, yZeros
divNext0a:
    vdivpd   yQuots, yOnes, yDivisors0
    vaddpd   ySums, ySums, yQuots
    vcmppd   yLess, yBorders, ySums, 2
    vptest   yLess, yLess
    jnz      divNext0b
    vaddpd   yDivisors0, yDivisors0, yOnes
    jmp      divNext0a
divNext0b:
    vmovdqa  [rdi + 256], ySums
    jmp      exitSumUpto

divNext1:
    MacroSumDivExpand yDivisors0, yDivisors0, 256, 0
    vcmppd   yLess, yBorders, ySums, 2
    vptest   yLess, yLess
    jnz      exitSumUpto

    vaddpd   yDivisors0, yDivisors0, yFours
    jmp      divNext1

divNext2:
    MacroSumDivExpand yDivisors0, yDivisors1, 256, 1
    MacroSumDivExpand yDivisors1, yDivisors0, 288, 0
    vcmppd   yLess, yBorders, ySums, 2
    vptest   yLess, yLess
    jnz      exitSumUpto

    vaddpd   yDivisors0, yDivisors1, yFours
    jmp      divNext2

divNext3:
    MacroSumDivExpand yDivisors0, yDivisors1, 256, 1
    MacroSumDivExpand yDivisors1, yDivisors2, 288, 1
    MacroSumDivExpand yDivisors2, yDivisors0, 320, 0
    vcmppd   yLess, yBorders, ySums, 2
    vptest   yLess, yLess
    jnz      exitSumUpto

    vaddpd   yDivisors0, yDivisors2, yFours
    jmp      divNext3

divNext4:
    MacroSumDivExpand yDivisors0, yDivisors1, 256, 1
    MacroSumDivExpand yDivisors1, yDivisors2, 288, 1
    MacroSumDivExpand yDivisors2, yDivisors3, 320, 1
    MacroSumDivExpand yDivisors3, yDivisors0, 352, 0
    vcmppd   yLess, yBorders, ySums, 2
    vptest   yLess, yLess
    jnz      exitSumUpto

    vaddpd   yDivisors0, yDivisors3, yFours
    jmp      divNext4

divNext5:
    MacroSumDivExpand yDivisors0, yDivisors1, 256, 1
    MacroSumDivExpand yDivisors1, yDivisors2, 288, 1
    MacroSumDivExpand yDivisors2, yDivisors3, 320, 1
    MacroSumDivExpand yDivisors3, yDivisors4, 352, 1
    MacroSumDivExpand yDivisors4, yDivisors0, 384, 0
    vcmppd   yLess, yBorders, ySums, 2
    vptest   yLess, yLess
    jnz      exitSumUpto

    vaddpd   yDivisors0, yDivisors4, yFours
    jmp      divNext5

divNext6:
    MacroSumDivExpand yDivisors0, yDivisors1, 256, 1
    MacroSumDivExpand yDivisors1, yDivisors2, 288, 1
    MacroSumDivExpand yDivisors2, yDivisors3, 320, 1
    MacroSumDivExpand yDivisors3, yDivisors4, 352, 1
    MacroSumDivExpand yDivisors4, yDivisors5, 384, 1
    MacroSumDivExpand yDivisors5, yDivisors0, 416, 0
    vcmppd   yLess, yBorders, ySums, 2
    vptest   yLess, yLess
    jnz      exitSumUpto

    vaddpd   yDivisors0, yDivisors5, yFours
    jmp      divNext6

divNext7:
    MacroSumDivExpand yDivisors0, yDivisors1, 256, 1
    MacroSumDivExpand yDivisors1, yDivisors2, 288, 1
    MacroSumDivExpand yDivisors2, yDivisors3, 320, 1
    MacroSumDivExpand yDivisors3, yDivisors4, 352, 1
    MacroSumDivExpand yDivisors4, yDivisors5, 384, 1
    MacroSumDivExpand yDivisors5, yDivisors6, 416, 1
    MacroSumDivExpand yDivisors6, yDivisors0, 448, 0
    vcmppd   yLess, yBorders, ySums, 2
    vptest   yLess, yLess
    jnz      exitSumUpto

    vaddpd   yDivisors0, yDivisors6, yFours
    jmp      divNext7

divNext8:
    MacroSumDivExpand yDivisors0, yDivisors1, 256, 1
    MacroSumDivExpand yDivisors1, yDivisors2, 288, 1
    MacroSumDivExpand yDivisors2, yDivisors3, 320, 1
    MacroSumDivExpand yDivisors3, yDivisors4, 352, 1
    MacroSumDivExpand yDivisors4, yDivisors5, 384, 1
    MacroSumDivExpand yDivisors5, yDivisors6, 416, 1
    MacroSumDivExpand yDivisors6, yDivisors7, 448, 1
    MacroSumDivExpand yDivisors7, yDivisors0, 480, 0
    vcmppd   yLess, yBorders, ySums, 2
    vptest   yLess, yLess
    jnz      exitSumUpto

    vaddpd   yDivisors0, yDivisors7, yFours
    jmp      divNext8

exitSumUpto:
    vmovdqa  [rdi + 0],   yDivisors0
    vmovdqa  [rdi + 32],  yDivisors1
    vmovdqa  [rdi + 64],  yDivisors2
    vmovdqa  [rdi + 96],  yDivisors3
    vmovdqa  [rdi + 128], yDivisors4
    vmovdqa  [rdi + 160], yDivisors5
    vmovdqa  [rdi + 192], yDivisors6
    vmovdqa  [rdi + 224], yDivisors7

    vmovdqa   ymm0,  ymmword ptr [rsi + 32]
    vmovdqa   ymm1,  ymmword ptr [rsi + 64]
    vmovdqa   ymm2,  ymmword ptr [rsi + 96]
    vmovdqa   ymm3,  ymmword ptr [rsi + 128]
    vmovdqa   ymm4,  ymmword ptr [rsi + 160]
    vmovdqa   ymm5,  ymmword ptr [rsi + 192]
    vmovdqa   ymm6,  ymmword ptr [rsi + 224]
    vmovdqa   ymm7,  ymmword ptr [rsi + 256]
    vmovdqa   ymm8,  ymmword ptr [rsi + 288]
    vmovdqa   ymm9,  ymmword ptr [rsi + 320]
    vmovdqa   ymm10, ymmword ptr [rsi + 352]
    vmovdqa   ymm11, ymmword ptr [rsi + 384]
    vmovdqa   ymm12, ymmword ptr [rsi + 416]
    vmovdqa   ymm13, ymmword ptr [rsi + 448]
    vmovdqa   ymm14, ymmword ptr [rsi + 480]
    vmovdqa   ymm15, ymmword ptr [rsi + 512]
    ret
