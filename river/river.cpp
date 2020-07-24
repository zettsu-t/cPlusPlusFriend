#include <iostream>
#include <type_traits>
#include <utility>

template <bool right>
struct Wolf {
    inline static constexpr bool r = right;
    using altType = typename Wolf::Wolf<!right>;
};

template <bool right>
struct Goat {
    inline static constexpr bool r = right;
    using altType = typename Goat::Goat<!right>;
};

template <bool right>
struct Cabbage {
    inline static constexpr bool r = right;
    using altType = typename Cabbage::Cabbage<!right>;
};

template <typename T, typename U, bool right>
struct eats {
    constexpr operator bool() const { return false; }
};

constexpr bool eats_impl(bool r1, bool r2, bool right) {
    return (r1 == r2) && (r1 != right);
};

template <bool r1, bool r2, bool right>
struct eats<Wolf<r1>, Goat<r2>, right> {
    constexpr operator bool() const { return eats_impl(r1, r2, right); }
};

template <bool r1, bool r2, bool right>
struct eats<Goat<r1>, Wolf<r2>, right> {
    constexpr operator bool() const { return eats_impl(r1, r2, right); }
};

template <bool r1, bool r2, bool right>
struct eats<Goat<r1>, Cabbage<r2>, right> {
    constexpr operator bool() const { return eats_impl(r1, r2, right); }
};

template <bool r1, bool r2, bool right>
struct eats<Cabbage<r1>, Goat<r2>, right> {
    constexpr operator bool() const { return eats_impl(r1, r2, right); }
};

template <typename Wtype, typename Gtype, typename Ctype, bool right, int prev_move>
struct State {
    using W = Wtype;
    using G = Gtype;
    using C = Ctype;
    inline static constexpr bool r=right;
    inline static constexpr bool m=prev_move;
    inline static constexpr bool goal = W::r & G::r & C::r & right;
};

template <typename S,
          typename W=typename S::W, typename G=typename S::G, typename C=typename S::C, bool right=S::r, int m=S::m,
          typename cond=std::enable_if_t<
              W::r==right && (m != (10 + !right)) && !S::goal &&
              !eats<typename W::altType, G, !right>() && !eats<G, C, !right>() &&
              !eats<C, typename W::altType, !right>(), std::nullptr_t>>
struct MoveWolf {
    using Next = typename ::State<typename W::altType, G, C, !right, 10 + right>;
};

template <typename S,
          typename W=typename S::W, typename G=typename S::G, typename C=typename S::C, bool right=S::r, int m=S::m,
          typename cond=std::enable_if_t<
              G::r==right && (m != (20 + !right)) && !S::goal &&
              !eats<W, typename G::altType, !right>() && !eats<typename G::altType, C, !right>() &&
              !eats<C, W, !right>(), std::nullptr_t>>
struct MoveGoat {
    using Next = typename ::State<W, typename G::altType, C, !right, 20 + right>;
};

template <typename S,
          typename W=typename S::W, typename G=typename S::G, typename C=typename S::C, bool right=S::r, int m=S::m,
          typename cond=std::enable_if_t<
              C::r==right && (m != (30 + !right)) && !S::goal &&
              !eats<W, G, !right>() && !eats<G, typename C::altType, !right>() &&
              !eats<typename C::altType, W, !right>(), std::nullptr_t>>
struct MoveCabbage {
    using Next = typename ::State<W, G, typename C::altType, !right, 30 + right>;
};

template <typename S,
          typename W=typename S::W, typename G=typename S::G, typename C=typename S::C, bool right=S::r, int m=S::m,
          typename cond=std::enable_if_t<
              !S::goal && !eats<W, G, !right>() && !eats<G, C, !right>() && !eats<C, W, !right>(),
              std::nullptr_t>>
struct MoveNothing {
    using Next = ::State<W, G, C, !right, 0>;
};

// Problems and answers cited from
// http://www.torito.jp/puzzles/106.shtml
int main(int argc, char* argv[]) {
    using s0 = State<Wolf<false>, Goat<false>, Cabbage<false>, false, 0>;
    using s1 = MoveGoat<s0>::Next;
    using s2 = MoveNothing<s1>::Next;
    using s3 = MoveWolf<s2>::Next;
    using s4 = MoveGoat<s3>::Next;
    using s5 = MoveCabbage<s4>::Next;
    using s6 = MoveNothing<s5>::Next;
    using s7 = MoveGoat<s6>::Next;
    std::cout << s7::goal;
    return 0;
}

static_assert(!eats<Wolf<false>, Goat<false>, false>());
static_assert(eats<Wolf<false>, Goat<false>, true>());
static_assert(eats<Wolf<true>, Goat<true>, false>());
static_assert(!eats<Wolf<true>, Goat<true>, true>());
static_assert(!eats<Wolf<false>, Goat<true>, false>());
static_assert(!eats<Wolf<true>, Goat<false>, true>());

static_assert(!eats<Goat<false>, Wolf<false>, false>());
static_assert(eats<Goat<false>, Wolf<false>, true>());
static_assert(eats<Goat<true>, Wolf<true>, false>());
static_assert(!eats<Goat<true>, Wolf<true>, true>());
static_assert(!eats<Goat<false>, Wolf<true>, false>());
static_assert(!eats<Goat<true>, Wolf<false>, true>());

static_assert(!eats<Goat<false>, Cabbage<false>, false>());
static_assert(eats<Goat<false>, Cabbage<false>, true>());
static_assert(eats<Goat<true>, Cabbage<true>, false>());
static_assert(!eats<Goat<true>, Cabbage<true>, true>());
static_assert(!eats<Goat<false>, Cabbage<true>, false>());
static_assert(!eats<Goat<true>, Cabbage<false>, true>());

static_assert(!eats<Cabbage<false>, Goat<false>, false>());
static_assert(eats<Cabbage<false>, Goat<false>, true>());
static_assert(eats<Cabbage<true>, Goat<true>, false>());
static_assert(!eats<Cabbage<true>, Goat<true>, true>());
static_assert(!eats<Cabbage<false>, Goat<true>, false>());
static_assert(!eats<Cabbage<true>, Goat<false>, true>());

static_assert(!eats<Wolf<false>, Cabbage<false>, false>());
static_assert(!eats<Wolf<true>, Cabbage<true>, true>());
static_assert(!eats<Wolf<false>, Cabbage<true>, false>());
static_assert(!eats<Wolf<true>, Cabbage<false>, true>());

static_assert(!eats<Cabbage<false>, Wolf<false>, false>());
static_assert(!eats<Cabbage<true>, Wolf<true>, true>());
static_assert(!eats<Cabbage<false>, Wolf<true>, false>());
static_assert(!eats<Cabbage<true>, Wolf<false>, true>());

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
