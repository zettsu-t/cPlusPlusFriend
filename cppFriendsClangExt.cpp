/* gccとclangを比較する */
#include "cppFriendsClang.hpp"

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

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
