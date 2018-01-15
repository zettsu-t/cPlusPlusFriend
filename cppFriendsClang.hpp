// GCCとclangで共通に使うヘッダファイル
#ifndef CPPFRIENDS_CPPFRIENDS_CLANG_HPP
#define CPPFRIENDS_CPPFRIENDS_CLANG_HPP

#include <cstdint>
#include <memory>
#include <sstream>

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

namespace ProcessorException {
    extern int32_t may_divide_by_zero(int32_t dividend, int32_t divisor, int32_t special);
    extern int abs_int(int src);
}

// LTOの有無で動作が変わる
extern uint32_t ShiftManyFor1(uint32_t src);

// Singletonコードの生成
class MySingletonClass {
public:
    static MySingletonClass& GetInstance(void);
    virtual int GetValue(void) const;
private:
    MySingletonClass(int a);
    virtual ~MySingletonClass(void) = default;
    int a_;
};

#endif // CPPFRIENDS_CPPFRIENDS_CLANG_HPP

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
