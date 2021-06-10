.intel_syntax noprefix
.file "suminv.s"

.set  gLoopIndex,   rbx
.set  gLoopIndex64, rbx

.set  yZeros,      ymm0
.set  yOnes,       ymm1
.set  yFours,      ymm2
.set  yBorders,    ymm3
.set  yWork,       ymm4
.set  yWork2,      ymm5
.set  yLess,       yWork2

.set  yDivisorsA,  ymm6
.set  yQuotsA,     ymm7
.set  ySumsA,      ymm8

.set  yDivisorsB,  ymm9
.set  yQuotsB,     ymm10
.set  ySumsB,      ymm11

.set  yDivisorsC,  ymm12
.set  yQuotsC,     ymm13
.set  ySumsC,      ymm14

.data
.align 32
data_zeros:    .long  0, 0, 0, 0
data_ones:     .long  1, 1, 1, 1
data_fours:    .long  4, 4, 4, 4
data_divisor:  .long  4, 3, 2, 1

.align 32
ymm_register_set:
    .quad 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
    .quad 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
    .quad 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
    .quad 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
    .quad 0

.text
.global sumUpto

.macro MacroDivSum quots, sums, work, work2
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

sumUpto:
    xor gLoopIndex64, gLoopIndex64
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
    vcvtdq2pd yDivisorsA, [rip + data_divisor]
    vcvtdq2pd ySumsA,     [rip + data_zeros]

divNext:
    vdivpd      yQuotsA, yOnes, yDivisorsA
    vaddpd      yDivisorsB, yDivisorsA, yFours
    vdivpd      yQuotsB, yOnes, yDivisorsB
    vaddpd      yDivisorsC, yDivisorsB, yFours
    vdivpd      yQuotsC, yOnes, yDivisorsC

    xor         gLoopIndex, gLoopIndex
    MacroDivSum yQuotsA, ySumsA, yWork, yWork2
    vcmppd      yLess, yBorders, ySumsA, 2
    vptest      yLess, yLess
    jnz         exitSum

    add         gLoopIndex, 1
    vpermq      ySumsB, ySumsA, 0
    MacroDivSum yQuotsB, ySumsB, yWork, yWork2
    vcmppd      yLess, yBorders, ySumsB, 1
    vptest      yLess, yLess
    jnz         exitSum

    add         gLoopIndex, 1
    vpermq      ySumsC, ySumsB, 0
    MacroDivSum yQuotsC, ySumsC, yWork, yWork2
    vcmppd      yLess, yBorders, ySumsC, 1
    vptest      yLess, yLess
    jnz         exitSum

    vpermq      ySumsA, ySumsC, 0
    vaddpd      yDivisorsA, yDivisorsC, yFours
    jmp         divNext

exitSum:
    vmovdqa  [rdi], yLess
    vmovdqa  [rdi + 32], yDivisorsA
    vmovdqa  [rdi + 64], yDivisorsB
    vmovdqa  [rdi + 96], yDivisorsC
    vmovdqa  [rdi + 128], ySumsA
    vmovdqa  [rdi + 160], ySumsB
    vmovdqa  [rdi + 192], ySumsC

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
