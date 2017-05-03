// ビルド方法
// このディレクトリ全体ではなく、このcppだけで実行するexeを作成するときは、以下のマクロを有効にする
// #define CPPFRIENDS_BUILD_STAND_ALONE

// MinGW GCC 6.3.0でコンパイルするときは、コマンドプロンプトから下記を実行する
// インクルードパスや実行ファイル名は適宜変更する
// g++ -std=gnu++14 -Wall -IC:\MinGW\include -o cppFriendsSpace cppFriendsSpace.cpp -lboost_regex

#include <codecvt>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <boost/regex.hpp>
// このディレクトリ全体ではなく、このcppだけで実行するexeを作成するときは不要
#ifndef CPPFRIENDS_BUILD_STAND_ALONE
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#endif

// 元記事
// https://twitter.com/kuina_ch/status/816977065480069121?lang=ja

namespace {
    // くいなちゃんが出した空白文字一覧(UTF-8)
    // 二番目は0xa0に戻してある
    const std::string spaceSet = "「 」「 」「 」「 」「 」「 」「 」「 」「 」「 」「 」「 」「 」「 」「 」「 」「　」";
    // 空白以外は一致しないことを確認する
    const std::string nonSpaceSet = "「.」「,」「x」「!」";
}

// このディレクトリ全体のテストを実行する
#ifndef CPPFRIENDS_BUILD_STAND_ALONE
class TestSpaceSet : public ::testing::Test{};

TEST_F(TestSpaceSet, Space) {
    std::ostringstream os;

    // くいなちゃんが出した空白文字一覧(UTF-16)
    auto spaceSet16 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(spaceSet);
    for(auto c : spaceSet16) {
        uint32_t code = static_cast<decltype(code)>(c);
        // 「」以外を表示する
        if ((c != 0x300c) && (c != 0x300d)) {
            os << std::hex << code << ":";
        }
    }

    // 正規表現パターン(POSIX文字クラスの空白:UTF-8)
    std::string pattern = "(「[[:space:]]」)";
    // 同UTF-16
    const auto pattern16 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(pattern);
    // 正規表現(UTF-16)
    boost::basic_regex<wchar_t> expr(pattern16.begin(), pattern16.end());

    // 「」区切りのイテレータ
    boost::regex_token_iterator<decltype(spaceSet16.begin()), wchar_t> i {spaceSet16.begin(), spaceSet16.end(), expr, 1};
    // 「」区切りのイテレータ(終端)
    decltype(i) e;

    // イテレータで捜査する
    using Code = uint32_t;
    std::unordered_map<Code, bool> codeMap;

    size_t count = 0;
    while(i != e) {
        const auto str16 = i->str();
        if (str16.size() < 3) {
            continue;
        }

        const auto code = static_cast<Code>(str16.at(1));
        if (codeMap.find(code) == codeMap.end()) {
            const auto s = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(str16);
            // UTF-8にしてから、文字と文字コードを表示する
            os << s << ":" << std::hex << code << "&";
            ++count;
        }

        codeMap[code] = true;
        ++i;
    }

    os << std::dec << count << " spaces found";

    // Cygwin GCC 5.4.0 の出力結果
    std::string expected = "20:a0:1680:2002:2003:2002:2003:2004:2005:2006:2007:2008:2009:200a:202f:205f:3000:「 」:20&「 」:1680&「 」:2002&「 」:2003&「 」:2004&「 」:2005&「 」:2006&「 」:2008&「 」:2009&「 」:200a&「 」:205f&「　」:3000&12 spaces found";
    EXPECT_EQ(expected, os.str());

    // 空白でなければ一致しないことを確認する
    auto nonSpaceSet16 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(nonSpaceSet);
    boost::regex_token_iterator<decltype(spaceSet16.begin()), wchar_t> in {nonSpaceSet16.begin(), nonSpaceSet16.end(), expr, 1};
    decltype(in) en;
    EXPECT_TRUE(in == en);
}

// このcppだけで実行するexeを作成するときは、以下のマクロを有効にする
#else

int main(int argc, char* argv[]) {
    // くいなちゃんが出した空白文字一覧(UTF-16)
    auto spaceSet16 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(spaceSet);
    for(auto c : spaceSet16) {
        uint32_t code = static_cast<decltype(code)>(c);
        // 「」以外を表示する
        if ((c != 0x300c) && (c != 0x300d)) {
            std::cout << std::hex << code << ":";
        }
    }

    // 正規表現パターン(POSIX文字クラスの空白:UTF-8)
    std::string pattern = "(「[[:space:]]」)";
    // 同UTF-16
    const auto pattern16 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(pattern);
    // 正規表現(UTF-16)
    boost::basic_regex<wchar_t> expr(pattern16.begin(), pattern16.end());

    // 「」区切りのイテレータ
    boost::regex_token_iterator<decltype(spaceSet16.begin()), wchar_t> i {spaceSet16.begin(), spaceSet16.end(), expr, 1};
    // 「」区切りのイテレータ(終端)
    decltype(i) e;

    // イテレータで捜査する
    using Code = uint32_t;
    std::unordered_map<Code, bool> codeMap;

    size_t count = 0;
    while(i != e) {
        const auto str16 = i->str();
        if (str16.size() < 3) {
            continue;
        }

        const auto code = static_cast<Code>(str16.at(1));
        if (codeMap.find(code) == codeMap.end()) {
            const auto s = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(str16);
            // UTF-8にしてから、文字と文字コードを表示する
            std::cout << s << ":" << std::hex << code << "&";
            ++count;
        }

        codeMap[code] = true;
        ++i;
    }

    // MinGW GCC 6.3.0は、Cygwin GCC 5.4.0と異なり、U+00a0, U+2007. U+202fも空白と扱う
    std::cout << "\n" << std::dec << count << " spaces found\n";

    // 空白でなければ一致しないことを確認する
    auto nonSpaceSet16 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(nonSpaceSet);
    boost::regex_token_iterator<decltype(spaceSet16.begin()), wchar_t> in {nonSpaceSet16.begin(), nonSpaceSet16.end(), expr, 1};
    decltype(in) en;
    bool skipped = (in == en);
    std::cout << (skipped ? "Non-space characters skipped\n" : "A non-space character captured!\n");
    return (skipped ? 0 : 1);
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
