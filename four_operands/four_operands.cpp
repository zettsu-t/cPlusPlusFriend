#include <cstring>
#include <iostream>
#include <memory>
#include <type_traits>

namespace {
    constexpr size_t XmmSize = 16;
    const float Vec1[4] {2.0, 3.0, 6.0, 7.0};
    const float Vec2[4] {19.0, 17.0, 13.0, 11.0};
    const float Zeros[4] {0.0};
    static_assert(sizeof(Vec1) == XmmSize);
    static_assert(sizeof(Vec2) == XmmSize);
    static_assert(sizeof(Zeros) == XmmSize);
}

void AlignedStorage(void) {
    // Len, Align の順に指定する。領域長は calloc (size*n bytes) とは違う。
    using XmmSrcMem = typename std::aligned_storage<XmmSize * 2, XmmSize>::type;
    XmmSrcMem xmmSrcMem;
    float* pXmmSrc = std::launder(reinterpret_cast<decltype(pXmmSrc)>(&xmmSrcMem));
    ::memmove(pXmmSrc, Vec1, sizeof(Vec1));
    ::memmove(pXmmSrc + 4, Vec2, sizeof(Vec2));

    using XmmDstMem = typename std::aligned_storage<XmmSize, XmmSize>::type;
    XmmDstMem xmmDstMem;
    float* pXmmDst = std::launder(reinterpret_cast<decltype(pXmmDst)>(&xmmDstMem));
    ::memmove(pXmmDst, Zeros, sizeof(Zeros));

    asm volatile (
        "mov rbx, 2 \n\t"
        "movaps xmm5, [rsi] \n\t"
        "vdpps xmm4, xmm5, ds:[rsi + rbx * 4 + 8], 0xd2 \n\t"
        "movaps [rdi], xmm4 \n\t"
        ::"S"(pXmmSrc),"D"(pXmmDst):"rbx", "xmm4", "xmm5", "memory");

    std::cout << pXmmSrc[0] << " : " << pXmmSrc[1] << " : " << pXmmSrc[2] << " : " << pXmmSrc[3] << "\n";
    std::cout << pXmmSrc[4] << " : " << pXmmSrc[5] << " : " << pXmmSrc[6] << " : " << pXmmSrc[7] << "\n";
    std::cout << pXmmDst[0] << " : " << pXmmDst[1] << " : " << pXmmDst[2] << " : " << pXmmDst[3] << "\n";
    std::cout << std::boolalpha << (pXmmDst[0] == 0.0) << ":" << (pXmmDst[1] == 193.0) <<
        ":" << (pXmmDst[2] == 0.0) << ":" << (pXmmDst[3] == 0.0) << "\n\n";
    return;
}

void AlignAsBytes(void) {
    // std::aligned_storageよりこちらの方がよい?
    alignas(XmmSize) std::byte xmmSrcMem[XmmSize * 2];
    float* pXmmSrc = std::launder(reinterpret_cast<decltype(pXmmSrc)>(xmmSrcMem));
    ::memmove(pXmmSrc, Vec1, sizeof(Vec1));
    ::memmove(pXmmSrc + 4, Vec2, sizeof(Vec2));

    alignas(XmmSize) std::byte xmmDstMem[XmmSize];
    float* pXmmDst = std::launder(reinterpret_cast<decltype(pXmmDst)>(xmmDstMem));
    ::memmove(pXmmDst, Zeros, sizeof(Zeros));

    asm volatile (
        "mov rbx, 2 \n\t"
        "movaps xmm5, [rsi] \n\t"
        "vdpps xmm4, xmm5, ds:[rsi + rbx * 4 + 8], 0xd2 \n\t"
        "movaps [rdi], xmm4 \n\t"
        ::"S"(pXmmSrc),"D"(pXmmDst):"rbx", "xmm4", "xmm5", "memory");

    std::cout << pXmmSrc[0] << " : " << pXmmSrc[1] << " : " << pXmmSrc[2] << " : " << pXmmSrc[3] << "\n";
    std::cout << pXmmSrc[4] << " : " << pXmmSrc[5] << " : " << pXmmSrc[6] << " : " << pXmmSrc[7] << "\n";
    std::cout << pXmmDst[0] << " : " << pXmmDst[1] << " : " << pXmmDst[2] << " : " << pXmmDst[3] << "\n";
    std::cout << std::boolalpha << (pXmmDst[0] == 0.0) << ":" << (pXmmDst[1] == 193.0) <<
        ":" << (pXmmDst[2] == 0.0) << ":" << (pXmmDst[3] == 0.0) << "\n\n";
    return;
}

void FourOperands(void) {
    alignas(XmmSize) std::byte xmmMem[XmmSize * 3];
    uint64_t* pXmm = std::launder(reinterpret_cast<decltype(pXmm)>(xmmMem));
    pXmm[0] = 0;
    pXmm[1] = 0;
    pXmm[2] = 2;
    pXmm[3] = 3;
    pXmm[4] = 5;
    pXmm[5] = 7;

//  3*5
//    11
//   00
//  11
//  1111 : +ではなくxorなのだが、たまたま同じ
    asm volatile (
        "movaps xmm5, [rdi + 16] \n\t"
        "movaps xmm6, [rdi + 32] \n\t"
        "vpclmulqdq xmm4, xmm5, xmm6, 0x1 \n\t"
        "movaps [rdi], xmm4 \n\t"
        ::"D"(pXmm):"xmm4", "xmm5", "xmm6", "memory");
    std::cout << pXmm[0] << " : " << pXmm[1] << "\n" << std::boolalpha <<
        (pXmm[0] == 15) << ":" << (pXmm[1] == 0) << "\n";

//  3*7
//    11
//   11
//  11
//  1001 : +ではなくxorなので
    asm volatile (
        "mov rbx, 2 \n\t"
        "movaps xmm5, [rdi + 16] \n\t"
        "vpclmulqdq xmm4, xmm5, ds:[rdi + rbx * 8 + 16], 0x11 \n\t"
        "movaps [rdi], xmm4 \n\t"
        ::"D"(pXmm):"rbx", "xmm4", "xmm5", "memory");
    std::cout << pXmm[0] << " : " << pXmm[1] << "\n" << std::boolalpha <<
        (pXmm[0] == 9) << ":" << (pXmm[1] == 0) << "\n\n";
    return;
}

int main(int argc, char* argv[]) {
    AlignedStorage();
    AlignAsBytes();
    FourOperands();
    return 0;
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
