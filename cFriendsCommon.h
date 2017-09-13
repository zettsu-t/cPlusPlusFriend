#ifndef CPPFRIENDS_CFRIENDSCOMMON_H
#define CPPFRIENDS_CFRIENDSCOMMON_H

#include <assert.h>
#include <string.h>
#include <stdlib.h>

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

    typedef int SpeedKph;  // 時速キロ

    // 速度の段階
    typedef enum tagSpeedIndex {
        SPEED_STOP,
        SPEED_LOW,
        SPEED_MIDDLE,
        SPEED_HIGH,   // もっと速い場合は読み替える
        SPEED_COUNT,  // 要素数
    } SpeedIndex;

    // 普通の速度指示
    typedef struct tagSpeedParameter {
        size_t     size;  // この構造体、またはこの構造体を含む構造体のbyteサイズ
        SpeedIndex index;
    } SpeedParameter;

    // 特に速い速度指示
    typedef struct tagExtSpeedParameter {
        SpeedParameter base;  // 必ず先頭メンバに置く
        SpeedKph   maxSpeed;
    } ExtSpeedParameter;

    static const size_t LongStringLength = 0xffffffff;
    static inline char* CreateLongString(void) {
        const size_t LongStringBufferLength = LongStringLength + 1;
        char* pStr = (char*)(malloc(sizeof(char) * LongStringBufferLength));
        assert(pStr);

        for(size_t i=0; i<LongStringLength; ++i) {
            pStr[i] = 'a';
        }
        pStr[LongStringLength] = '\0';
        return pStr;
    }

    static inline int IsLongStringEmpty(void) {
        char* pStr = CreateLongString();
        int result = (strlen(pStr) == 0);
        free(pStr);
        pStr = NULL;
        return result;
    }

    static inline size_t GetLongStringLength(void) {
        char* pStr = CreateLongString();
        size_t length = strlen(pStr);
        free(pStr);
        pStr = NULL;
        return length;
    }

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
