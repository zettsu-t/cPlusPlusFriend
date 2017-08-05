#include <gtest/gtest.h>
#include <boost/version.hpp>

#ifdef BOOST_NO_EXCEPTIONS
#include <boost/throw_exception.hpp>
namespace boost {
    void throw_exception(std::exception const& e) {
        std::terminate();
    }
}
#endif

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
