#include <cstdint>
#include <iostream>

template <typename T> requires std::integral<T> T succ(const T from) {
  return from + static_cast<T>(1);
}

int main(int argc, char *argv[]) {
  constexpr uint16_t value = 1;
  std::cout << succ(value) << "\n";
  std::cout << succ(static_cast<long long>(2)) << "\n";
  int32_t n = 3;
  std::cout << succ(n) << "\n";
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
