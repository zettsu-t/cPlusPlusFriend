#include <iostream>
#include "cmp_bool.hpp"

int main(int argc, char* argv[]) {
    constexpr MyBool falseValue = MyBool::False;
    constexpr MyBool trueValue = MyBool::True;

    std::cout << "isFalse(falseValue) " << isFalse(falseValue) << "\n";
    std::cout << "isFalse(falseValue) " << isFalse(trueValue) << "\n";
    std::cout << "isTrue(trueValue) " << isTrue(trueValue) << "\n";
    std::cout << "isTrue(trueValue) " << isTrue(falseValue)<< "\n";

    std::cout << "isNotFalse(falseValue) " << isNotFalse(falseValue) << "\n";
    std::cout << "isNotFalse(falseValue) " << isNotFalse(trueValue) << "\n";
    std::cout << "isNotTrue(trueValue) " << isNotTrue(trueValue) << "\n";
    std::cout << "isNotTrue(trueValue) " << isNotTrue(falseValue)<< "\n";

    return 0;
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
