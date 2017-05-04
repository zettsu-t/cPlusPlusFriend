#include <stdio.h>
#include "cFriends.h"

// 定義が見えていないときのコードを.sで確認する
void exec_io_with_declaration(void) {
    char buf[64];

    for(int i = 0; i < 3; ++i) {
        // これはvolatileなので、毎回メモリから値を読みに行く
        snprintf(buf, sizeof(buf), "%64d", g_memoryMappedClock);
        // これはvolatileではないので、"xor r9d, r9d" で定数0が入る!
        snprintf(buf, sizeof(buf), "%64d", g_nonVolatileClock);
    }

    return;
}

int get_elapsed_time(void) {
    int first = g_nonVolatileClock;
    // この間に何か処理をする
#ifdef CPPFRIENDS_SIDE_EFFECT
    printf("!");
#endif
    // printfがあると、g_nonVolatileClockを読み直す
    // printfがないと、xor eax, eax に最適化されて、0が返る!
    int second = g_nonVolatileClock;
    return second - first;
}

/*
Local Variables:
mode: c
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
