#ifndef CPPFRIENDS_CPPFRIENDS_HPP
#define CPPFRIENDS_CPPFRIENDS_HPP

#include <cstdint>

// int32を32回以上シフトする実験
#define CPPFRIENDS_SHIFT_COUNT (35)

// result = RCXレジスタを破壊することを兼ねて敢えてこのような引数にしている
extern void Shift35ForInt32Asm(int32_t& result, int32_t src);

// int32を32回以上シフトする実験
// シフト回数を固定する
extern int32_t Shift35ForInt32(int32_t src);
// シフト回数を実行時に与える
extern int32_t ShiftForInt32(int32_t src, int32_t count);

#endif // CPPFRIENDS_CPPFRIENDS_HPP

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
