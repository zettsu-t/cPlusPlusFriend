#include <cstdio>
#include <iostream>
#include <memory>

auto make_counter_dangling(int init) {
    auto count = init;
    return [&]() {
        return count++;
    };
}

auto make_counter(int init) {
    auto box = std::make_unique<int>(init);
    return [count = std::move(box)]() {
        return (*count)++;
    };
}

void run() {
    auto counter1 = make_counter(12);
    auto counter2 = make_counter(234);
    printf("%d\n", counter1());
    printf("%d\n", counter2());
    printf("%d\n", counter1());
    printf("%d\n", counter2());
    printf("\n");
}

void run_dangling1() {
    auto counter1 = make_counter_dangling(34);
    auto counter2 = make_counter_dangling(456);
    const auto value1_1 = counter1();
    const auto value2_1 = counter2();
    const auto value1_2 = counter1();
    const auto value2_2 = counter2();
    printf("%d\n", value1_1);
    printf("%d\n", value2_1);
    printf("%d\n", value1_2);
    printf("%d\n", value2_2);
    printf("\n");
}

void run_dangling2() {
    auto counter1 = make_counter_dangling(56);
    auto counter2 = make_counter_dangling(678);
    printf("%d\n", counter1());
    printf("%d\n", counter2());
    printf("%d\n", counter1());
    printf("%d\n", counter2());
    printf("\n");
}

int main(int argc, char* argv[]) {
    run();
    run_dangling1();
    run_dangling2();
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
