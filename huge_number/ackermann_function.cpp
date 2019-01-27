// Ackermann function
// https://mathtrain.jp/ackermann
#include <iostream>

template<int M, int N>
struct Ackermannfunction {
    static constexpr int value =
        Ackermannfunction<M - 1,
                          Ackermannfunction<M, N - 1>::value
                          >::value;
};

template<int N>
struct Ackermannfunction<0, N> {
    static constexpr int value = N + 1;
};

template<int M>
struct Ackermannfunction<M, 0> {
    static constexpr int value = Ackermannfunction<M - 1, 1>::value;
};

// Set M and N
int main(int argc, char* argv[]) {
    constexpr int M = 3;
    constexpr int N = 3;
    std::cout << Ackermannfunction<M, N>::value << "\n";
    return 0;
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
