#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "cFriends.h"
#include "cFriendsCommon.h"

#ifndef static_assert
#define static_assert _Static_assert
#endif

// 定義が見えていたらどうだろう?
volatile int g_memoryMappedClock;  // レジスタから値を読みに行くイメージ
int g_nonVolatileClock;            // レジスタから値を読みに行くが、うまくいかないイメージ

static_assert(sizeof(char) == 1, "Expect sizeof(char) == 1");
// Cではこうなる。C++ではこうならない。
static_assert(sizeof('a') == sizeof(int), "Expect sizeof('a') == sizeof(int)");
static_assert(sizeof(g_arrayForTestingSize) == sizeof(int), "g_arrayForTestingSize must have more than one elements");
static_assert(sizeof(TestingEmptyStruct) == 0, "Expect sizeof(empty struct) == 0");

// 桁落ちを意図的に起こす
void check_cancellation_digits(void) {
    // x87なら内部80bitで、SSEなら64bitで計算する
    double narrow = 0.0;
    // x87が内部80bitで計算したものを、強制的にメモリに書き出すことで精度を64bitにする
    // -ffloat-storeでコンパイルすると常にこうする
    volatile double volatileNarrow = 0.0;
    static_assert(sizeof(narrow) == sizeof(volatileNarrow), "Must be same size");

    double diff = 1.0;
    for(int i=0; i<128; ++i) {
        narrow += diff;
        volatileNarrow += diff;
        diff /= 3.1;
    }

    const char* pResult = (narrow == volatileNarrow) ? "equal to" : "different from";  // 浮動小数の比較は警告が出る
    printf("The non-volatile double value is %s the volatile\n", pResult);
}

void my_memcpy(uint8_t* restrict pDst, const uint8_t* restrict pSrc, size_t size) {
    for(size_t i = 0; i < size; ++i) {
        pDst[i] = pSrc[i];
    }

    return;
}

#define MY_ARRAY_SIZE (5)

void exec_my_memcpy(void) {
    uint8_t src[MY_ARRAY_SIZE] = {2,3,4,5,6};
    uint8_t dst[MY_ARRAY_SIZE] = {0,0,0,0,0};
    size_t size = MY_ARRAY_SIZE;

    check_cancellation_digits();
    my_memcpy(dst, src, size);
    for(size_t i = 0; i < size; ++i) {
        printf("%u:", (unsigned int)dst[i]);
    }

    // このdo-whileは最適化されてなくなるので、ループ実行のオーバヘッドはなくなるという実験
    do {
        printf("\n");
    } while(0);
    return;
}

void print_infinity(void) {
    double f = -1.0;
    for(int i= 0; i < 308; ++i) {
        f /= 10.00001;
    }
    printf("f=%.0e\n", f);

    f = -1.0;
    for(int i= 0; i < 309; ++i) {
        f *= 10.00001;
    }
    printf("f=%.0e\n", f);
    printf("f=-infinity\n");
    return;
}

void exec_snprintf(void) {
    char buffer[4] = "END";    // 壊れているかどうか後で調べる
    char dst[8];               // 転送先
    char src[9] = "12345678";  // 転送元
    static_assert(sizeof(buffer) < sizeof(dst), "Wrong size setting");
    static_assert(sizeof(dst) < sizeof(src), "Wrong size setting");

    // 下の変数ほど番地が大きいと仮定しているが、そうでないかもしれない
    ptrdiff_t diff = src - dst;
    if (diff < 0) {
        diff = buffer - dst;
    }
    diff -= (ptrdiff_t)(sizeof(dst));  // 符号が取れるのでキャストがないと警告が出る
    printf("gap for dst:%td bytes\n", diff);

    snprintf(dst, sizeof(dst), "%s", src);
    printf("snprintf:%s\n", dst);

    strncpy(dst, buffer, sizeof(dst));
    strncpy(dst, src, sizeof(dst));
    dst[sizeof(dst) - 1] = 0;  // これがないとdstがNUL終端されない
    printf("strncpy:%s.%s\n", dst, buffer);
    return;
}

void exec_snprintf_twice(void) {
    // 文字数を測る
    const char src[8] = "0123456";    // 転送元
    char buf[10]      = "ABCDEFGHI";  // 壊れているかどうか後で調べる

#ifdef __MINGW64__
    // 非0を渡す必要がある
    int size = snprintf(buf, 0, "%s", src);
    // このsizeにNUL終端は含まない
    fprintf(stderr, "%s <<<\n", buf);
    // 十分なバッファがあれば書き込むはずの文字数が返ってくるはずだが...
    assert(size == -1);
    size = sizeof(src) - 1;
#else
    // 最初の引数にNULLを渡しても大丈夫か?
//  int size = snprintf(NULL, 0, "%s", src);
    int size = snprintf(buf, 0, "%s", src);
    // このsizeにNUL終端は含まない
    assert((size + 1) == sizeof(src));
#endif
    assert(buf[sizeof(src)-2] == 'G');

    size_t requiredSize = (size_t)(size + 1);  // 符号が取れるのでキャストがないと警告が出る
    size = snprintf(buf, requiredSize, "%s", src);
    assert((size + 1) == sizeof(src));
    assert(buf[sizeof(src)-2] == '6');
    assert(buf[sizeof(src)-1] == '\0');
    assert(buf[sizeof(buf)-2] == 'I');
    printf("%s\n", buf);
}

void exec_io_with_definition(void) {
    static volatile int memoryMappedClock;
    static int nonVolatileClock;
    char buf[64];

    for(int i = 0; i < 3; ++i) {
        // これはvolatileなので、毎回メモリから値を読みに行く
        snprintf(buf, sizeof(buf), "%64d", g_memoryMappedClock);
        // これはvolatileではないが、snprintfが書き換えたかもしれないので毎回メモリから値を読みに行く
        snprintf(buf, sizeof(buf), "%64d", g_nonVolatileClock);
        // これはvolatileなので、毎回メモリから値を読みに行く
        snprintf(buf, sizeof(buf), "%64d", memoryMappedClock);
        // これはvolatileではないので、メモリは読まず、"xor r9d, r9d"で定数0が入る!
        snprintf(buf, sizeof(buf), "%64d", nonVolatileClock);
    }
    return;
}

// 負の数に対する除算に対して、コンパイラがどんなコードを出力するか確認する
int divide_by_2(int src) {
    return src / 2;
}

int main(int argc, char* argv[]) {
    exec_my_memcpy();
    exec_snprintf();
    exec_snprintf_twice();
    print_infinity();

    DWORD startTime = GetTickCount();
    char* pStr = CreateLongString();
    free(pStr);
    pStr = NULL;

    DWORD emptyTime = GetTickCount();
    assert(!IsLongStringEmpty());
    DWORD lengthTime = GetTickCount();
    assert(GetLongStringLength() == LongStringLength);
    DWORD stopTime = GetTickCount();

    printf("%lu, %lu, %lu [msec]\n",
           (unsigned long)(emptyTime  - startTime),
           (unsigned long)(lengthTime - emptyTime),
           (unsigned long)(stopTime   - lengthTime));
    return 0;
}

/*
Local Variables:
mode: c
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
