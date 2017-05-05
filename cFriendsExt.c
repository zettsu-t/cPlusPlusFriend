#include <stdio.h>
#include "cFriends.h"

int get_elapsed_time_and_inline(void) {
    int first = g_nonVolatileClock;
    // この間に何か処理をする
#ifdef CPPFRIENDS_SIDE_EFFECT
    do_something();
#endif
    // do_somethingの定義が見えていないと、g_nonVolatileClockを読み直す
    // do_somethingの定義が見えていると、xor eax, eax に最適化されて、0が返る!
    int second = g_nonVolatileClock;
    return second - first;
}

// printfの定義は見えないので、
// グローバル変数を書き換えた疑いがあることを前提に最適化する
int get_elapsed_time_printf(void) {
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
