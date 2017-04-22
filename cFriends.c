#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static_assert(sizeof(char) == 1, "Expect sizeof(char) == 1");
// Cではこうなる。C++ではこうならない。
static_assert(sizeof('a') == sizeof(int), "Expect sizeof('a') == sizeof(int)");

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

    const char* pResult = (narrow == volatileNarrow) ? "equal to" : "different from";
    printf("The non-volatile double value is %s the volatile\n", pResult);
}

void my_memcpy(uint8_t* restrict pDst, const uint8_t* restrict pSrc, size_t size) {
    for(size_t i = 0; i < size; ++i) {
        pDst[i] = pSrc[i];
    }

    return;
}

#define MY_ARRAY_SIZE (5)

int main(int argc, char* argv[]) {
    uint8_t src[MY_ARRAY_SIZE] = {2,3,4,5,6};
    uint8_t dst[MY_ARRAY_SIZE] = {0,0,0,0,0};
    size_t size = MY_ARRAY_SIZE;

    check_cancellation_digits();
    my_memcpy(dst, src, size);
    for(size_t i = 0; i < size; ++i) {
        printf("%u:", (unsigned int)dst[i]);
    }

    printf("\n\n");
    return 0;
}
