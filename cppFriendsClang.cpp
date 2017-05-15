/* gccとclangを比較する */
#include <cstdint>
#include <cstring>
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
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
