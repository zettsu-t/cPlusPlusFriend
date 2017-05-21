#include <assert.h>
#include <stdint.h>
#include <stdio.h>

// gcc, clangでは「複数行のコメント」警告が出る

int main(int argc, char* argv[]) {
    // Shift_JISでこのコメントはまずいでゲソ
    printf("First line\n");

    // -Wallなし、つまりで警告なしでこの問題を見抜くのは不可能
    printf("Second line\n");

    // ではないと思いますが、-Wall -Werror をつけるべきだと思うのです
    printf("Third line\n");
    return 0;
}

/*
Local Variables:
mode: c
coding: shift_jis-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
