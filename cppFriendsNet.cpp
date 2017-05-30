// やめるのだフェネックで学ぶC++の実証コード(TCP/UDP/IP)
#include <cstring>
#include <iostream>
#include <vector>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/type_traits/function_traits.hpp>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <gtest/gtest.h>
#include "cFriendsCommon.h"
#include "cppFriends.hpp"

// UDP/TCP受信コードはコンパイルしないようにしています
// 動作を試すときだけ有効にしてください
#if 0
class TestResourceLeakage : public ::testing::Test {
protected:
    virtual void TearDown() override {
        clear();
    }

    void clear() {
        for(auto& fd : fds_) {
            ::close(fd);
        }
        fds_.clear();
    }

    std::vector<decltype(::socket(0,0,0))> fds_;
};

TEST_F(TestResourceLeakage, Leak) {
    const bool CloseSocket[] = {true, false};
    constexpr decltype(fds_)::size_type count = 3;
    constexpr boost::function_traits<decltype(htons)>::arg1_type portNumber = 13577;

    for(auto closeSocket : CloseSocket) {
        for(decltype(fds_)::size_type i=1; i<=count; ++i) {
            auto fd = ::socket(AF_INET, SOCK_DGRAM, 0);
            ASSERT_LE(0, fd);
            fds_.push_back(fd);

            struct sockaddr_in addr;
            ::memset(&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = ::inet_addr("127.0.0.1");  // localhostではうまくいかないことがある
            addr.sin_port = htons(portNumber);

            auto result = ::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
            if ((i == 1) || closeSocket) {
                EXPECT_FALSE(result);
            } else {
                EXPECT_TRUE(result);
            }

            if (closeSocket) {
                fds_.pop_back();
                ::close(fd);
                EXPECT_TRUE(fds_.empty());
            }
        }

        if (closeSocket) {
            EXPECT_TRUE(fds_.empty());
        } else {
            EXPECT_FALSE(fds_.empty());
        }
        clear();
        EXPECT_TRUE(fds_.empty());
    }
}

// http://www.boost.org/doc/libs/1_64_0/doc/html/boost_asio/tutorial/tutdaytime1.html
// http://www.boost.org/doc/libs/1_64_0/doc/html/boost_asio/tutorial/tutdaytime2.html
// を元にしています
// このコードを試すには、下記のとおりブラウザからアクセスするのが手頃です
TEST_F(TestResourceLeakage, DoNotLeak) {
    constexpr size_t count = 3;

    for(size_t i=1; i<=count; ++i) {
        constexpr unsigned short portNumber = 13579;
        boost::asio::io_service ioService;
        // localhostしか受け付けない
        boost::asio::ip::tcp::acceptor acceptor(ioService,
                                                boost::asio::ip::tcp::endpoint(
                                                    boost::asio::ip::address::from_string("127.0.0.1"),
                                                    portNumber));
        boost::asio::ip::tcp::socket socket(ioService);
        acceptor.accept(socket);

        // Webブラウザから、http://localhost:13579/ にアクセスすると、
        // HTTPヘッダが表示される。リロードすると、セッションが切れる。
        for(;;) {
            boost::array<char, 16> buf;  // バッファが短くても長いものを読める
            boost::system::error_code errorCode;
            auto len = socket.read_some(boost::asio::buffer(buf), errorCode);
            if (errorCode || errorCode ==  boost::asio::error::eof) {
                break;
            }
            std::cout.write(buf.data(), len);
        }

        // 明示的にcloseしなくてもTCPセッションを閉じる
    }
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
