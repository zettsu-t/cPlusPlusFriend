#include "cmp_bool.hpp"

bool isFalse(MyBool value) {
    return (value == MyBool::False);
}

bool isTrue(MyBool value) {
    return (value == MyBool::True);
}

bool isNotFalse(MyBool value) {
    return (value != MyBool::False);
}

bool isNotTrue(MyBool value) {
    return (value != MyBool::True);
}

bool toBool(int cond) {
    return (cond) ? true : false;
}

bool f(bool cond) {
    return (cond) ? true : false;
}

int mySelect(bool cond, int t, int f) {
    return (cond) ? t : f;
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
