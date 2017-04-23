#include <assert.h>
#include <stdint.h>
#include <stdio.h>

// gcc, clangでは「複数行のコメント」警告が出る

int main(int argc, char* argv[]) {
    // Shift_JISでこのコメントはまずいでゲソ
    printf("First line\n");

    // -Wallなし、つまりで警告なしでこの問題を見抜くのは不可能
    printf("Second line\n");

    // ではないだと思いますが、-Wall -Werrorすべきだと思うのです
    printf("Third line\n");
    return 0;
}
