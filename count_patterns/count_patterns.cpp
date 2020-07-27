// Count Yu.*Ka.*Ta.*Yu in YuKaTaYuTaYuTaYu
// This quiz is cited from
// https://twitter.com/yukata_yu/status/1286061070004531201
// Run this online in
// https://wandbox.org/permlink/DDp0RUdwpjceZqxr

#include <iostream>
#include <type_traits>

struct Yu {};
struct Ka {};
struct Ta {};
struct Nil {};

struct NilImpl {
    inline constexpr static int count = 0;
};

template <template <typename Ts> class U, typename... Ts>
struct EndOfWordImpl : public U<Ts...> {
    inline constexpr static int count = U<Ts...>::count + 1;
};

template <template <typename Ts> class U, template <typename Ts> class V, typename... Ts>
struct PairImpl : public U<Ts...>, public V<Ts...> {
    inline constexpr static int count = U<Ts...>::count + V<Ts...>::count;
};

template <typename T, bool matched, bool is_nil, typename... Ts>
struct YuFirstImpl;

template <typename T, typename... Ts>
struct YuFirst : public YuFirstImpl<T, std::is_same_v<T, Yu>, std::is_same_v<T, Nil>, Ts...> {};

template <typename T, bool matched, bool is_nil, typename... Ts>
struct KaElementImpl;

template <typename T, typename... Ts>
struct KaElement : public KaElementImpl<T, std::is_same_v<T, Ka>, std::is_same_v<T, Nil>, Ts...> {};

template <typename T, bool matched, bool is_nil, typename... Ts>
struct TaElementImpl;

template <typename T, typename... Ts>
struct TaElement : public TaElementImpl<T, std::is_same_v<T, Ta>, std::is_same_v<T, Nil>, Ts...> {};

template <typename T, bool matched, bool is_nil, typename... Ts>
struct YuLastImpl;

template <typename T, typename... Ts>
struct YuLast : public YuLastImpl<T, std::is_same_v<T, Yu>, std::is_same_v<T, Nil>, Ts...> {};

template <typename T, typename... Ts>
struct YuFirstImpl<T, true, false, Ts...> : public PairImpl<YuFirst, KaElement, Ts...> {};

template <typename T, typename... Ts>
struct YuFirstImpl<T, false, true, Ts...> : public NilImpl {};

template <typename T, typename... Ts>
struct YuFirstImpl<T, false, false, Ts...> : public YuFirst<Ts...> {};

template <typename T, typename... Ts>
struct KaElementImpl<T, true, false, Ts...> : public PairImpl<KaElement, TaElement, Ts...> {};

template <typename T, typename... Ts>
struct KaElementImpl<T, false, true, Ts...> : public NilImpl {};

template <typename T, typename... Ts>
struct KaElementImpl<T, false, false, Ts...> : public KaElement<Ts...> {};

template <typename T, typename... Ts>
struct TaElementImpl<T, true, false, Ts...> : public PairImpl<TaElement, YuLast, Ts...> {};

template <typename T, typename... Ts>
struct TaElementImpl<T, false, true, Ts...> : public NilImpl {};

template <typename T, typename... Ts>
struct TaElementImpl<T, false, false, Ts...> : public TaElement<Ts...> {};

template <typename T, typename... Ts>
struct YuLastImpl<T, true, false, Ts...> : public EndOfWordImpl<YuLast, Ts...> {};

template <typename T, typename... Ts>
struct YuLastImpl<T, false, true, Ts...> : public NilImpl {};

template <typename T, typename... Ts>
struct YuLastImpl<T, false, false, Ts...> : public YuLast<Ts...> {};

template<typename... Ts>
using YuCounter = YuFirst<Ts..., Nil>;

int main(void) {
    // Count matches of /Yu.*Ka.*Ta.*Yu/  in YuKaTaYuTaYuTaYu
    YuCounter<Yu, Ka, Ta, Yu, Ta, Yu, Ta, Yu> pattern;
    std::cout << "Found " << pattern.count << " pattern(s).\n";
    std::cout << sizeof(pattern) << " bytes.\n";
    return 0;
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
Last:
*/
