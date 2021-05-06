#include <cstdint>
#include <memory>
#include <iostream>

class Box {
public:
    Box(int x) : x_(x) {}
    virtual ~Box() = default;
    void Print() {
        std::cout << x_ << "\n";
    }
private:
    int x_ {0};
};

std::unique_ptr<Box> Create(int x) {
    return std::make_unique<Box>(x);
}

int main(void) {
    std::unique_ptr<Box> p = std::move(Create(1));
    p->Print();
    std::unique_ptr<Box> q(Create(1));
    q->Print();

    using Num = int;
    constexpr bool y = true;
    constexpr Num x = static_cast<decltype(x)>(y);
    std::cout << x << " " << y << "\n";

    bool b = false;
    Num a = static_cast<decltype(a)>(b);
    std::cout << a << " " << b << "\n";
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
