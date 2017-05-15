#ifndef CPPFRIENDS_CPPFRIENDS_HPP
#define CPPFRIENDS_CPPFRIENDS_HPP

#include <cstdint>
#include <type_traits>
#include <boost/type_traits/function_traits.hpp>

// int32を32回以上シフトする実験
#define CPPFRIENDS_SHIFT_COUNT (35)

// result = RCXレジスタを破壊することを兼ねて敢えてこのような引数にしている
extern void Shift35ForInt32Asm(int32_t& result, int32_t src);

// int32を32回以上シフトする実験
// シフト回数を固定する
extern int32_t Shift35ForInt32(int32_t src);
// シフト回数を実行時に与える
extern int32_t ShiftForInt32(int32_t src, int32_t count);

// 定義した.cppとは別の.cppから参照される
// ヘッダで定義すると定数伝搬できるが、定数を変えた時に、
// インクルードしている.cppをすべてビルドしなければならない
extern const int g_externIntValue;

namespace {
    // ある.cppでは使うが、他の.cppでは使わないインライン関数
    inline int MyPopCount(short src) {
        using T = boost::function_traits<decltype(__builtin_popcount)>::arg1_type;
        static_assert(std::is_convertible<decltype(src), T>::value, "cannot convert");
        return __builtin_popcount(src);
    }

//  __attribute__((always_inline))
    int MySlowPopCount(unsigned int src) {
        return (src) ? ((src & 1) + MySlowPopCount(src >> 1)) : 0;
    }
}

// enum classは前方宣言できるので、メンバを増やしてもリコンパイルが必要とは限らない
enum class FriendType : int;

struct FriendTypeBox {
    FriendType type_;  // underlying typeがあればメンバは知らなくてよい
    // 他に必要なメンバを追加する
};

#endif // CPPFRIENDS_CPPFRIENDS_HPP

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
