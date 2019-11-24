.intel_syntax noprefix
.file "popcnt.s"

.set  BytesInYmmReg, 32

.data
.align BytesInYmmReg
ymmRegArray: .quad 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
             .quad 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
             .quad 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
             .quad 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
             .quad 0,0,0,0

generalRegArray: .quad 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0

.set  yRegMask0, ymm10
.set  yRegMask1, ymm11
.set  yRegMask2, ymm12
.set  yRegMask3, ymm13
.set  yRegMask4, ymm14
.set  yRegMaskCount, ymm15

.set  xRegMask0, xmm10
.set  xRegMask1, xmm11
.set  xRegMask2, xmm12
.set  xRegMask3, xmm13
.set  xRegMask4, xmm14
.set  xRegMaskCount, xmm15

.set  yRegWork0, ymm0
.set  yRegWork1, ymm1
.set  yRegWork2, ymm2
.set  yRegWork3, ymm3
.set  yRegWork4, ymm4
.set  yRegWork5, ymm5
.set  yRegWork6, ymm6
.set  yRegWork7, ymm7
.set  yRegSum,   ymm8

.set  xRegWork0, xmm0
.set  xRegWork1, xmm1
.set  xRegWork2, xmm2
.set  xRegWork3, xmm3
.set  xRegWork4, xmm4
.set  xRegWork5, xmm5
.set  xRegWork6, xmm6
.set  xRegWork7, xmm7
.set  xRegSum,   xmm8

## Save in callers
.set  regWorkQ,   r15
.set  regWorkD,   r15d
## Arguments
.set  regSrcAddr, rsi
.set  regLoop,    rcx
## Return value
.set  regSumQ,    rax
.set  regSumD,    eax

.macro SaveYmmRegiser yRegSrc, slot
    vmovdqa [rip + ymmRegArray + \slot * BytesInYmmReg], \yRegSrc
.endm

.macro LoadYmmRegiser yRegDst, slot
    vmovdqa \yRegDst, [rip + ymmRegArray + \slot * BytesInYmmReg]
.endm

.macro FillYmmRegiser xRegDst, yRegDst, regWorkD, numD
    mov      \regWorkD, \numD
    vpinsrd  \xRegDst, \xRegDst, \regWorkD, 0
    vpshufd  \xRegDst, \xRegDst, 0
    vpbroadcastq \yRegDst, \xRegDst
.endm

## Destroys regSrc
.macro ConvoluteYmmBits regDst, regSrc, regMask, width
    vpsrld  \regDst, \regSrc, \width
    vpand   \regDst, \regDst, \regMask
    vpand   \regSrc, \regSrc, \regMask
    vpaddd  \regDst, \regDst, \regSrc
.endm

## Returns the population count regB of in either regA or regB
## Destroys both registers
.macro ConvoluteYmmRegister regA, regB
    ConvoluteYmmBits \regA, \regB, yRegMask0, 1
    ConvoluteYmmBits \regB, \regA, yRegMask1, 2
    ConvoluteYmmBits \regA, \regB, yRegMask2, 4
    ConvoluteYmmBits \regB, \regA, yRegMask3, 8
#   ConvoluteYmmBits \regA, \regB, yRegMask4, 16
.endm

.macro CountPopulationInYmmRegister yRegA, yRegB
    ConvoluteYmmRegister \yRegA, \yRegB
    ## 128 * 2 -> 64 * 2 -> 32 * 2 -> 16 * 2
    vphaddw \yRegB, \yRegB, \yRegB
    vphaddw \yRegB, \yRegB, \yRegB
    vphaddw \yRegB, \yRegB, \yRegB
    ## Zero extend word to qword
    vandps  \yRegB, yRegMaskCount, \yRegB
    vpaddq  yRegSum, yRegSum, \yRegB
.endm

.text
.global countPopulation
countPopulation:
    ## This is not thread-safe because using a fixed address to store registers
    SaveYmmRegiser ymm0, 0
    SaveYmmRegiser ymm1, 1
    SaveYmmRegiser ymm2, 2
    SaveYmmRegiser ymm3, 3
    SaveYmmRegiser ymm4, 4
    SaveYmmRegiser ymm5, 5
    SaveYmmRegiser ymm6, 6
    SaveYmmRegiser ymm7, 7
    SaveYmmRegiser ymm8, 8
    SaveYmmRegiser ymm9, 9
    SaveYmmRegiser ymm10, 10
    SaveYmmRegiser ymm11, 11
    SaveYmmRegiser ymm12, 12
    SaveYmmRegiser ymm13, 13
    SaveYmmRegiser ymm14, 14
    SaveYmmRegiser ymm15, 15
    mov     [rip + generalRegArray], regWorkQ

    vxorps  yRegMaskCount, yRegMaskCount, yRegMaskCount
    vxorps  yRegSum, yRegSum, yRegSum
    xor     regSumQ, regSumQ

    FillYmmRegiser xRegMask0, yRegMask0, regWorkD, 0x55555555
    FillYmmRegiser xRegMask1, yRegMask1, regWorkD, 0x33333333
    FillYmmRegiser xRegMask2, yRegMask2, regWorkD, 0x0f0f0f0f
    FillYmmRegiser xRegMask3, yRegMask3, regWorkD, 0x00ff00ff
    FillYmmRegiser xRegMask4, yRegMask4, regWorkD, 0x0000ffff

    ## A bitmask for 9-bit population count (0..256) of one 256-bit YMM register
    mov      regWorkD, 0xffff
    vpinsrd  xRegMaskCount, xRegMaskCount, regWorkD, 0
    vpbroadcastq yRegMaskCount, xRegMaskCount

1:
    vmovdqa yRegWork0, [regSrcAddr]
    vmovdqa yRegWork2, [regSrcAddr + BytesInYmmReg]
    vmovdqa yRegWork4, [regSrcAddr + BytesInYmmReg * 2]
    vmovdqa yRegWork6, [regSrcAddr + BytesInYmmReg * 3]
    CountPopulationInYmmRegister yRegWork1, yRegWork0
    CountPopulationInYmmRegister yRegWork3, yRegWork2
    CountPopulationInYmmRegister yRegWork5, yRegWork4
    CountPopulationInYmmRegister yRegWork7, yRegWork6
    add     regSrcAddr, BytesInYmmReg * 4
    sub     rcx, 4
    ## The LOOP instruction is slow and cannot jump short to 1:
    jg      1b

    vpextrq regSumQ, xRegSum, 0
    vextracti128 xRegWork0, yRegSum, 1
    vpextrq regWorkQ, xRegWork0, 0
    add     regSumQ, regWorkQ

    LoadYmmRegiser ymm0, 0
    LoadYmmRegiser ymm1, 1
    LoadYmmRegiser ymm2, 2
    LoadYmmRegiser ymm3, 3
    LoadYmmRegiser ymm4, 4
    LoadYmmRegiser ymm5, 5
    LoadYmmRegiser ymm6, 6
    LoadYmmRegiser ymm7, 7
    LoadYmmRegiser ymm8, 8
    LoadYmmRegiser ymm9, 9
    LoadYmmRegiser ymm10, 10
    LoadYmmRegiser ymm11, 11
    LoadYmmRegiser ymm12, 12
    LoadYmmRegiser ymm13, 13
    LoadYmmRegiser ymm14, 14
    LoadYmmRegiser ymm15, 15
    mov     regWorkQ, [rip + generalRegArray]
    ret
