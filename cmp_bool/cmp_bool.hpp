#ifndef CMP_BOOl_HPP
#define CMP_BOOl_HPP

enum class MyBool {
    False,
    True
};

extern bool isFalse(MyBool value);
extern bool isTrue(MyBool value);
extern bool isNotFalse(MyBool value);
extern bool isNotTrue(MyBool value);
extern bool toBool(int cond);
extern bool f(bool cond);
extern int mySelect(bool cond, int t, int f);

#endif // CMP_BOOl_HPP

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
