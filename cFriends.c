#include <stdint.h>
#include <stdio.h>

void my_memcpy(uint8_t* restrict pDst, const uint8_t* pSrc, size_t size) {
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

    my_memcpy(dst, src, size);
    for(size_t i = 0; i < size; ++i) {
        printf("%u:", (unsigned int)dst[i]);
    }

    printf("\n");
    return 0;
}
