/* gccとclangを比較する */
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <random>
#include "cppFriendsClang.hpp"

namespace SwitchCase {
    // 呼び出し先をインライン展開させない
    double GetFixedTestValue(SwitchCase::Shape shape) {
        double actual = 0.0;
        switch(shape) {
        case SwitchCase::Shape::CIRCLE:
            actual = SwitchCase::GetAreaOfCircle(2.0);
            break;
        case SwitchCase::Shape::RECTANGULAR:
            actual = SwitchCase::GetAreaOfRectangular(2.0, 3.0);
            break;
        case SwitchCase::Shape::TRIANGLE:
            actual = SwitchCase::GetAreaOfTriangle(6.0, 8.0, 10.0);
            break;
            // caseが4個だと、clangはルックアップテーブルを作る
        case SwitchCase::Shape::SQUARE:
            actual = SwitchCase::GetAreaOfRectangular(7.0, 7.0);
            break;
        default:
            break;
        }

        return actual;
    }
}

namespace MemoryOperation {
    uint32_t g_shortBuffer[2];
    uint32_t g_largeBuffer[1024];

    void FillByZeroShort(void) {
        // mov qword ptr [rax], 0
        ::memset(g_shortBuffer, 0, sizeof(g_shortBuffer));
    }

    void FillByZeroLarge(void) {
        // rex64 jmp memset # TAILCALL
        ::memset(g_largeBuffer, 0, sizeof(g_largeBuffer));
    }

    struct BitFields {
        unsigned int member1 : 2;
        unsigned int member2 : 4;
        unsigned int member3 : 4;
        unsigned int member4 : 5;
        unsigned int : 0;
    };

    void SetBitFields(BitFields& fields) {
        fields.member1 = 1;
        fields.member2 = 3;
        fields.member3 = 7;
        fields.member4 = 15;
        // 11' 1101'1100'1101b = 15,821
        return;
   }

    void SetBitFieldsAtOnce(BitFields& fields) {
        const BitFields origin = {1, 3, 7, 15};
        fields = origin;
        return;
   }

#if 0
    // g++ -std=gnu++14 -Wall -IC:\MinGW\include
    // -O2 -S -masm=intel -mavx2 -o cppFriendsClang.s cppFriendsClang.cpp
    // 両関数ともこのコードになる
    movzx   eax, BYTE PTR [rcx]
    and eax, -64
    or  eax, 13
    mov BYTE PTR [rcx], al
    movzx   eax, WORD PTR [rcx]
    and ax, -961
    or  ax, 448
    mov WORD PTR [rcx], ax
    shr ax, 8
    and eax, -125
    or  eax, 60
    mov BYTE PTR 1[rcx], al
    ret

    // clang++ -std=gnu++14 -Wall --target=x86_64-pc-windows-gnu
    // -isystem C:\MinGW\include -isystem C:\MinGW\lib\gcc\x86_64-w64-mingw32\6.3.0\include
    // -O2 -S -masm=intel -mavx2 -o cppFriendsClang.s cppFriendsClang.cpp
    // SetBitFields
    movzx   eax, word ptr [rcx]
    and eax, 32768
    or  eax, 15821
    mov word ptr [rcx], ax
    ret
    // SetBitFieldsAtOnce
    mov dword ptr [rcx], 15821
    ret
#endif
}

namespace Devirtualization {
    std::string GetStringInline(void) {
        std::ostringstream os;
        BaseInline baseObj;
        DerivedInline derivedObj;

        // オブジェクトの型が分かっているので、vtableを観ずに直接呼び出す
        // 定義が見えているのでインライン展開も行う
        // L.str:
        // .asciz  "BaseInline"
        // movabs  rdx, L.str
        // mov  r8d, 10
        // mov  rcx, rdi
        // call  ...ostream_insert...
        baseObj.Print(os);
        derivedObj.Print(os);
        std::string str = os.str();
        return str;
    }

    std::string GetStringOutline(void) {
        std::ostringstream os;
        auto pBase = CreateBaseOutline();
        auto pDerived = CreateDerivedOutline();

        // オブジェクトの型が分かっていないので、vtableを経由して直接呼び出す
        // mov  rcx, qword ptr [rsp + 48]
        // mov  rax, qword ptr [rcx]
        // call qword ptr [rax + 8]
        pBase->Print(os);
        pDerived->Print(os);
        std::string str = os.str();
        return str;
    }
}

namespace ConditionalMove {
    void condMove(uint_fast32_t seed, bool expr, uint_fast32_t& fst, uint_fast32_t& snd) {
        std::mt19937 gen(seed);
        uint_fast32_t n = gen();
        // これはconditional moveで済む
        fst = (expr) ? 1 : n;
        // これはどちらを呼び出すかどうか分けなければならない
        snd = (expr) ? gen() : static_cast<uint_fast32_t>(::time(nullptr));
        return;
    }

    // Sudoku Rustてきなコード(分岐あり)
    void SudokuCell::filter_by_candidates_1(const SudokuCell& rhs) {
        auto new_candidates = has_unique_candidate() ? MaskBits : ~rhs.candidates_;
        candidates_ &= new_candidates;
    }

    // Sudoku Rustてきなコード(分岐なし)
    void SudokuCell::filter_by_candidates_2(const SudokuCell& rhs) {
        auto new_candidates = has_no_or_unique_candidates() ? MaskBits : ~rhs.candidates_;
        candidates_ &= new_candidates;
    }

    bool SudokuCell::has_unique_candidate(void) {
        return (candidates_ > 0) && ((candidates_ & (candidates_ - 1)) == 0);
    }

    bool SudokuCell::has_no_or_unique_candidates(void) {
        return ((candidates_ & (candidates_ - 1)) == 0);
    }
}

namespace ProcessorException {
    // 0で割れないのは有名だが、負で絶対値が最大の数を-1で割っても正の整数の範囲には収まらない
    int32_t may_divide_by_zero(int32_t dividend, int32_t divisor, int32_t special) {
        return (divisor) ? (dividend / divisor) : special;
    }

    // 負で絶対値が最大の数は、正の整数の範囲には収まらない
    int abs_int(int src) {
        return std::abs(src);
    }
}

namespace CheckWarning {
    int WrongInterator(const std::vector<int>& vec) {
        const auto size = vec.size();
        int value = 0;

        for(auto i = decltype(size){0}; i < size; ++i) {
            // これはコンパイラに警告して欲しい
            for(i = 0; i < size; ++i) {
                const auto& v = vec.at(i);
                if (v < 0) {
                    value = v;
                }
            }
        }

        return value;
    }
}

namespace {
    template <typename T,
              std::enable_if_t<std::is_integral<T>::value, std::nullptr_t> = nullptr>
    class MulDiv {
    public:
        using Int = typename std::make_signed<T>::type;
        using Uint = typename std::make_unsigned<T>::type;
        static_assert(sizeof(T) == sizeof(Int), "Must keep its bit width");
        static_assert(sizeof(T) == sizeof(Uint), "Must keep its bit width");
        static_assert(std::is_signed<Int>::value, "Must be signed");
        static_assert(std::is_unsigned<Uint>::value, "Must be unsigned");

        static Int MulInt(Int a, Int b) {
            return a * b;
        }

        static Uint MulUint(Uint a, Uint b) {
            return a * b;
        }

        static Int DivInt(Int dividend, Int divider) {
            return dividend / divider;
        }

        static Uint DivUint(Uint dividend, Uint divider) {
            return dividend / divider;
        }

        static std::pair<Int, Int> DivRemInt(Int dividend, Int divider) {
            return std::pair<Int, Int> {dividend / divider, dividend % divider};
        }

        static std::pair<Uint, Uint> DivRemUint(Uint dividend, Uint divider) {
            return std::pair<Uint, Uint> {dividend / divider, dividend % divider};
        }
    };
}

int8_t MulInt8(int8_t dividend, int8_t divider) {
    return MulDiv<int8_t>::MulInt(dividend, divider);
}

uint8_t MulUint8(uint8_t dividend, uint8_t divider) {
    return MulDiv<uint8_t>::MulUint(dividend, divider);
}

int8_t DivInt8(int8_t dividend, int8_t divider) {
    return MulDiv<int8_t>::DivInt(dividend, divider);
}

uint8_t DivUint8(uint8_t dividend, uint8_t divider) {
    return MulDiv<uint8_t>::DivUint(dividend, divider);
}

std::pair<int8_t, int8_t> DivRemInt8(int8_t dividend, int8_t divider) {
    return MulDiv<int8_t>::DivRemInt(dividend, divider);
}

std::pair<uint8_t, uint8_t> DivRemUint8(uint8_t dividend, uint8_t divider) {
    return MulDiv<uint8_t>::DivRemUint(dividend, divider);
}


int16_t MulInt16(int16_t dividend, int16_t divider) {
    return MulDiv<int16_t>::MulInt(dividend, divider);
}

uint16_t MulUint16(uint16_t dividend, uint16_t divider) {
    return MulDiv<uint16_t>::MulUint(dividend, divider);
}

int16_t DivInt16(int16_t dividend, int16_t divider) {
    return MulDiv<int16_t>::DivInt(dividend, divider);
}

uint16_t DivUint16(uint16_t dividend, uint16_t divider) {
    return MulDiv<uint16_t>::DivUint(dividend, divider);
}

std::pair<int16_t, int16_t> DivRemInt16(int16_t dividend, int16_t divider) {
    return MulDiv<int16_t>::DivRemInt(dividend, divider);
}

std::pair<uint16_t, uint16_t> DivRemUint16(uint16_t dividend, uint16_t divider) {
    return MulDiv<uint16_t>::DivRemUint(dividend, divider);
}


int32_t MulInt32(int32_t dividend, int32_t divider) {
    return MulDiv<int32_t>::MulInt(dividend, divider);
}

uint32_t MulUint32(uint32_t dividend, uint32_t divider) {
    return MulDiv<uint32_t>::MulUint(dividend, divider);
}

int32_t DivInt32(int32_t dividend, int32_t divider) {
    return MulDiv<int32_t>::DivInt(dividend, divider);
}

uint32_t DivUint32(uint32_t dividend, uint32_t divider) {
    return MulDiv<uint32_t>::DivUint(dividend, divider);
}

std::pair<int32_t, int32_t> DivRemInt32(int32_t dividend, int32_t divider) {
    return MulDiv<int32_t>::DivRemInt(dividend, divider);
}

std::pair<uint32_t, uint32_t> DivRemUint32(uint32_t dividend, uint32_t divider) {
    return MulDiv<uint32_t>::DivRemUint(dividend, divider);
}


int64_t MulInt64(int64_t dividend, int64_t divider) {
    return MulDiv<int64_t>::MulInt(dividend, divider);
}

uint64_t MulUint64(uint64_t dividend, uint64_t divider) {
    return MulDiv<uint64_t>::MulUint(dividend, divider);
}

int64_t DivInt64(int64_t dividend, int64_t divider) {
    return MulDiv<int64_t>::DivInt(dividend, divider);
}

uint64_t DivUint64(uint64_t dividend, uint64_t divider) {
    return MulDiv<uint64_t>::DivUint(dividend, divider);
}

std::pair<int64_t, int64_t> DivRemInt64(int64_t dividend, int64_t divider) {
    return MulDiv<int64_t>::DivRemInt(dividend, divider);
}

std::pair<uint64_t, uint64_t> DivRemUint64(uint64_t dividend, uint64_t divider) {
    return MulDiv<uint64_t>::DivRemUint(dividend, divider);
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
