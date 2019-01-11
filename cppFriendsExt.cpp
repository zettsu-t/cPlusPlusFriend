#include <cstring>
#include "cppFriends.hpp"

// インライン展開されると結果が変わる関数をここに書く

void Shift35ForInt32Asm(int32_t& result, int32_t src) {
    int32_t count = CPPFRIENDS_SHIFT_COUNT;
    asm volatile (
        "movl %1,   %0 \n\t"
        "movl %2,   %%ecx \n\t"
        "shl  %%cl, %0 \n\t"
        :"=r"(result):"r"(src),"r"(count):"%ecx","memory");
    // 第一引数はRCXレジスタに入っているので、破壊レジスタに指定しないとクラッシュする
//      :"=r"(result):"r"(src),"r"(count):);
}

int32_t Shift35ForInt32(int32_t src) {
    decltype(src) result = src;
// シフト回数が多すぎる警告を出さないようにする
#if CPPFRIENDS_SHIFT_COUNT >= 32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshift-count-overflow"
#endif
    result <<= CPPFRIENDS_SHIFT_COUNT;
// 以後は、シフト回数が多すぎる警告を出す
#if CPPFRIENDS_SHIFT_COUNT >= 32
#pragma GCC diagnostic pop
#endif
    return result;
}

int32_t ShiftForInt32(int32_t src, int32_t count) {
    decltype(src) result = src;
    result <<= count;
    return result;
}

// 宣言と定義の型が違ったら困る例
// うろ覚えの情報で、int*だと思ってアクセスされると、
// 0xf0000000 または 0xe0000000f0000000を指すポインタとして読まれてしまう
unsigned int g_pointerOrArray[] = {0xf0000000u, 0xe0000000u};

// 他の.cppから参照される
const int g_externIntValue = 1;

// クラスにこのようなことをすると、vtableへのポインタもクリアしてしまう
void SubDynamicObjectMemFunc::Clear(void) {
    memset(this, 0, sizeof(*this));
}

void SubDynamicObjectMemFunc::Print(std::ostream& os) {
    os << memberA_ << memberB_;
}

void ExtraMemFunc::Print(std::ostream& os) {
    os << "Extra";
}

MyStringList::MyStringList(size_t n) {
    for(decltype(n) i=0; i<n; ++i) {
        dataSet_.push_back("12345678901234567890123456789012345678901234567890123456789012345");
    }
}

void MyStringList::Print(std::ostream& os) const {
    for(auto& s : dataSet_) {
        os << s;
    }
}

void MyStringList::Pop(void) {
    dataSet_.pop_front();
}

void MyStringList::Clear(void) {
    dataSet_.clear();
}

namespace {
    const int ConstantIntSample = 12;
}

const uint8_t ConstantArraySample[ConstantArraySampleSize] {0};
const int& ConstantIntRefSample = ConstantIntSample;

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
