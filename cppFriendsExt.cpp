#include "cppFriends.hpp"

void Shift35ForInt32(int32_t& result, int32_t src) {
    asm volatile (
        "movl %1,   %0 \n\t"
        "movl $35,  %%ecx \n\t"
        "shl  %%cl, %0 \n\t"
       :"=r"(result):"r"(src):"%ecx");
    // 第一引数はRCXレジスタに入っているので、破壊レジスタに指定しないとクラッシュする
    // :"=r"(result):"r"(src):);
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
