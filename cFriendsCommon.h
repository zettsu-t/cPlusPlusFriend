#ifndef CPPFRIENDS_CFRIENDSCOMMON_H
#define CPPFRIENDS_CFRIENDSCOMMON_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// CとC++のどちらでコンパイルするかでサイズが変わる
static const char g_arrayForTestingSize[sizeof('a')] = {'A'};

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
