/* gccとclangを比較する */
#define _USE_MATH_DEFINES
#include <cmath>
#include "cppFriendsClang.hpp"

namespace SwitchCase {
    double GetAreaOfCircle(double radius) {
        return radius * radius * M_PI;
    }

    double GetAreaOfRectangular(double width, double height) {
        return width * height;
    }

    // ヘロンの公式
    double GetAreaOfTriangle(double edge1, double edge2, double edge3) {
        double halfSum = edge1 + edge2 + edge3;
        halfSum /= 2.0;
        double product = halfSum * (halfSum - edge1) * (halfSum - edge2) * (halfSum - edge3);
        return std::sqrt(product);
    }
}

namespace Devirtualization {
    void BaseOutline::Print(std::ostream& os) {
        os << "BaseOutline";
        return;
    }

    void DerivedOutline::Print(std::ostream& os) {
        os << "DerivedOutline";
        return;
    }

    std::unique_ptr<BaseOutline> CreateBaseOutline(void) {
        return std::unique_ptr<BaseOutline>(new BaseOutline());
    }

    std::unique_ptr<BaseOutline> CreateDerivedOutline(void) {
        return std::unique_ptr<BaseOutline>(new DerivedOutline());
    }
}

// LTOで削除する
int UnusedFunction(void) {
    return 0;
}

// LTOだと動作が変わる。未定義動作だから仕方ない。
uint32_t ShiftManyFor1(uint32_t src) {
    uint32_t i = 1;
    return (i << src);
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
