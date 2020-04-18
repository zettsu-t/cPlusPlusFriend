// GCCとclangで共通に使うヘッダファイル
#ifndef CPPFRIENDS_CPPFRIENDS_CLANG_HPP
#define CPPFRIENDS_CPPFRIENDS_CLANG_HPP

#include <cstdint>
#include <memory>
#include <sstream>
#include <type_traits>
#include <utility>

// switch-caseから呼ばれる関数。インライン展開させない。
namespace SwitchCase {
    enum class Shape {
        UNKNOWN,
        CIRCLE,
        RECTANGULAR,
        TRIANGLE,
        SQUARE,
    };
    // cppFriendsSample1.cppで定義する
    double GetAreaOfCircle(double radius);
    double GetAreaOfRectangular(double width, double height);
    double GetAreaOfTriangle(double edge1, double edge2, double edge3);
    // cppFriendsClang.cppで定義する
    double GetFixedTestValue(SwitchCase::Shape shape);
}

namespace Devirtualization {
    // 定義が呼び出し側から見える
    class BaseInline {
    public:
        virtual ~BaseInline(void) = default;
        virtual void Print(std::ostream& os) {
            os << "BaseInline";
            return;
        }
    };

    class DerivedInline : public BaseInline {
    public:
        virtual ~DerivedInline(void) = default;
        virtual void Print(std::ostream& os) override {
            os << "DerivedInline";
            return;
        }
    };

    // 定義は呼び出し側から見えない
    class BaseOutline {
    public:
        virtual ~BaseOutline(void) = default;
        virtual void Print(std::ostream& os);
    };

    class DerivedOutline : public BaseOutline {
    public:
        virtual ~DerivedOutline(void) = default;
        virtual void Print(std::ostream& os) override;
    };

    // 具象型が何かは呼び出し側から見えない
    extern std::unique_ptr<BaseOutline> CreateBaseOutline(void);
    extern std::unique_ptr<BaseOutline> CreateDerivedOutline(void);

    extern std::string GetStringInline(void);
    extern std::string GetStringOutline(void);
}

namespace ConditionalMove {
    using SudokuCandidates = unsigned int;
    struct SudokuCell {
        SudokuCandidates candidates_ {0};
        void filter_by_candidates_1(const SudokuCell& rhs);
        void filter_by_candidates_2(const SudokuCell& rhs);
        bool has_unique_candidate(void);
        bool has_no_or_unique_candidates(void);
    };
    static constexpr SudokuCandidates MaskBits = 0x1ffu;
}

namespace ProcessorException {
    extern int32_t may_divide_by_zero(int32_t dividend, int32_t divisor, int32_t special);
    extern int abs_int(int src);
}

namespace InferVariadicTemplate {
    template <typename F, typename... V>
    auto MyApply(const F& f, V&&... v) {
        return f(std::forward<V>(v)...);
    };

#if !defined(__clang__)
    template <typename F>
    auto MyApply2(const F& f, auto&&... v) {
        return f(std::forward<decltype(v)>(v)...);
    };
#endif
}

// LTOの有無で動作が変わる
extern uint32_t ShiftManyFor1(uint32_t src);

// 乗除算のコードを確認する
extern int8_t MulInt8(int8_t dividend, int8_t divider);
extern uint8_t MulUint8(uint8_t dividend, uint8_t divider);
extern int8_t DivInt8(int8_t dividend, int8_t divider);
extern uint8_t DivUint8(uint8_t dividend, uint8_t divider);
extern std::pair<int8_t, int8_t> DivRemInt8(int8_t dividend, int8_t divider);
extern std::pair<uint8_t, uint8_t> DivRemUint8(uint8_t dividend, uint8_t divider);

extern int16_t MulInt16(int16_t dividend, int16_t divider);
extern uint16_t MulUint16(uint16_t dividend, uint16_t divider);
extern int16_t DivInt16(int16_t dividend, int16_t divider);
extern uint16_t DivUint16(uint16_t dividend, uint16_t divider);
extern std::pair<int16_t, int16_t> DivRemInt16(int16_t dividend, int16_t divider);
extern std::pair<uint16_t, uint16_t> DivRemUint16(uint16_t dividend, uint16_t divider);

extern int32_t MulInt32(int32_t dividend, int32_t divider);
extern uint32_t MulUint32(uint32_t dividend, uint32_t divider);
extern int32_t DivInt32(int32_t dividend, int32_t divider);
extern uint32_t DivUint32(uint32_t dividend, uint32_t divider);
extern std::pair<int32_t, int32_t> DivRemInt32(int32_t dividend, int32_t divider);
extern std::pair<uint32_t, uint32_t> DivRemUint32(uint32_t dividend, uint32_t divider);

extern int64_t MulInt64(int64_t dividend, int64_t divider);
extern uint64_t MulUint64(uint64_t dividend, uint64_t divider);
extern int64_t DivInt64(int64_t dividend, int64_t divider);
extern uint64_t DivUint64(uint64_t dividend, uint64_t divider);
extern std::pair<int64_t, int64_t> DivRemInt64(int64_t dividend, int64_t divider);
extern std::pair<uint64_t, uint64_t> DivRemUint64(uint64_t dividend, uint64_t divider);

#endif // CPPFRIENDS_CPPFRIENDS_CLANG_HPP

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
