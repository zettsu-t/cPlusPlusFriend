#include <iostream>
#include <type_traits>

class Int {
    friend std::ostream& operator<<(std::ostream&, const Int&);
public:
    // The explicit keyword is required on either the ctor or the conversion constructor below.
    constexpr explicit Int(int x) : m_(x) {}
    constexpr Int operator +(const Int& r) const { return Int(m_ + r.m_); }
    constexpr operator int() const { return m_; }
private:
    int m_ {0};
};

std::ostream& operator<<(std::ostream& os, const Int& value) {
    os << value.m_;
    return os;
}

int main(int argc, char* argv[]) {
    auto x = Int(1) + Int(2);
    static_assert(std::is_same_v<decltype(x), Int>);
    auto y = Int(3) + 5;
    static_assert(std::is_same_v<decltype(y), int>);

    std::cout << x << "\n";
    std::cout << y;
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
