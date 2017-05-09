#ifndef CPPFRIENDS_CFRIENDS_H
#define CPPFRIENDS_CFRIENDS_H

extern volatile int g_memoryMappedClock;  // レジスタから値を読みに行くイメージ
extern int g_nonVolatileClock;            // レジスタから値を読みに行くが、うまくいかないイメージ

#ifdef CPPFRIENDS_SIDE_EFFECT
// get_elapsed_time_and_inlineから定義が見えないので、
// グローバル変数を書き換えたと仮定する
extern void do_something(void);
#else
// get_elapsed_time_and_inlineから定義が見えるので、
// グローバル変数を書き換えていないと分かる
inline void do_something(void) {
    return;
}
#endif

#endif // CPPFRIENDS_CFRIENDS_H

/*
Local Variables:
mode: c
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
