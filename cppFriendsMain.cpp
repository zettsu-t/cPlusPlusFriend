#include <gtest/gtest.h>
#include <boost/version.hpp>

int main(int argc, char* argv[]) {
    std::cout << "Run with Boost C++ Libraries " << (BOOST_VERSION / 100000) << "." << (BOOST_VERSION / 100 % 1000);
    std::cout << "." << (BOOST_VERSION % 100) << "\n";

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
