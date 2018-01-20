#include <boost/fusion/container/vector.hpp>
#include <gtest/gtest.h>

// Singletonコードの生成
class MySingletonClass {
public:
    static MySingletonClass& GetInstance(void);
    virtual int GetValue(void) const;
private:
    MySingletonClass(int a);
    virtual ~MySingletonClass(void) = default;
    int a_;
};

// このstatic変数を作ったかどうかのフラグはatomicではない
MySingletonClass& MySingletonClass::GetInstance(void) {
    static MySingletonClass instance(1);
    return instance;
}

int MySingletonClass::GetValue(void) const {
    return a_;
}

MySingletonClass::MySingletonClass(int a) : a_(a) {}

class TestMySingletonTest : public ::testing::Test {};

TEST_F(TestMySingletonTest, All) {
    MySingletonClass& instance1 = MySingletonClass::GetInstance();
    EXPECT_EQ(1, instance1.GetValue());
    MySingletonClass& instance2 = MySingletonClass::GetInstance();
    EXPECT_EQ(&instance1, &instance2);
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
