// C++20 or later required
#include <sstream>
#include <gtest/gtest.h>

#if __cplusplus >= 202002L

template <class T>
concept Printable = requires (const T& x) {
    x.to_string();
};

template <Printable T>
void printObject(std::ostream& os, const T& x) {
    os << x.to_string();
}

template <typename T>
void printObject(std::ostream& os, const T& x) {
    os << "?";
}

namespace {
    class PrintableClass {
    public:
        PrintableClass(const std::string& name) : name_(name) {
        }

        const std::string to_string(void) const {
            return name_;
        };
    private:
        std::string name_;
    };

    struct NonPrintableClass {};
}

class TestConcept : public ::testing::Test {};

TEST_F(TestConcept, PrintableConcept) {
    std::ostringstream os;
    PrintableClass aPrintable("Foo");
    printObject(os, aPrintable);
    EXPECT_EQ("Foo", os.str());

    NonPrintableClass aNonPrintable;
    printObject(os, aNonPrintable);
    EXPECT_EQ("Foo?", os.str());
}

#endif

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
