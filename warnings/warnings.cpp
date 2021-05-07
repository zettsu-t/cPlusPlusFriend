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

void moveUnique(void) {
    std::unique_ptr<Box> p = std::move(Create(1));
    p->Print();
}

void constructUnique(void) {
    std::unique_ptr<Box> p(Create(2));
    p->Print();
}

void moveToShared(void) {
    std::shared_ptr<Box> p = std::move(Create(3));
    p->Print();
}

void constructShared(void) {
    std::shared_ptr<Box> p(Create(4));
    p->Print();
}

void decltypeCast(void) {
    using Num = int;
    constexpr bool y = true;
    constexpr Num x = static_cast<decltype(x)>(y);
    std::cout << x << " " << y << "\n";

    bool b = false;
    Num a = static_cast<decltype(a)>(b);
    std::cout << a << " " << b << "\n";
}

int main(void) {
    moveUnique();
    constructUnique();
    moveToShared();
    constructShared();
    decltypeCast();
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
