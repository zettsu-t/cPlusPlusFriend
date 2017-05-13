// GCCとclangで共通に使うヘッダファイル
#ifndef CPPFRIENDS_CPPFRIENDS_CLANG_HPP
#define CPPFRIENDS_CPPFRIENDS_CLANG_HPP

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

#endif // CPPFRIENDS_CPPFRIENDS_CLANG_HPP

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
