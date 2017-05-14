#ifndef CPPFRIENDS_CFRIENDSCOMMON_H
#define CPPFRIENDS_CFRIENDSCOMMON_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    // CとC++のどちらでコンパイルするかでサイズが変わる
    static const char g_arrayForTestingSize[sizeof('a')] = {'A'};

    typedef struct tagTestingOuterStruct1 {
        struct tagTestingInnerStruct {
            int member1;
        } anInstance;
        int member;
    } TestingOuterStruct1;

    typedef struct tagTestingOuterStruct2 {
        // C++では、OuterStruct1の内部クラスとは区別される
        // Cではstruct tagInnerStructを再定義したことになり、エラーになる
#if 0
        struct tagTestingInnerStruct {
            int member2;
        } anInstance;
#endif
        int member;
    } TestingOuterStruct2;

    // 空の構造体のサイズが、gccとg++で異なる
    // そもそもCで空の構造体は作れない(未定義動作)
    typedef struct tagTestingEmptyStruct {} TestingEmptyStruct;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // CPPFRIENDS_CFRIENDSCOMMON_H

/*
Local Variables:
mode: c
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
