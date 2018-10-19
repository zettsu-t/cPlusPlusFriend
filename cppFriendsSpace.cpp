// ビルド方法
// このディレクトリ全体ではなく、このcppだけで実行するexeを作成するときは、以下のマクロを有効にする
// #define CPPFRIENDS_REGEX_BUILD_STAND_ALONE

// MinGW GCC 6.3.0でコンパイルするときは、コマンドプロンプトから下記を実行する
// インクルードパスや実行ファイル名は適宜変更する
// chcp 932でもchcp 65001でも、実行結果の表示がおかしい
// g++ -std=gnu++14 -Wall -IC:\MinGW\include -DCPPFRIENDS_REGEX_BUILD_STAND_ALONE -o cppFriendsSpace cppFriendsSpace.cpp -lboost_regex
// cppFriendsSpace.bat を実行してもよい

#include <codecvt>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <boost/regex.hpp>
#include <boost/version.hpp>

// このディレクトリ全体ではなく、このcppだけで実行するexeを作成するときは不要
#ifndef CPPFRIENDS_REGEX_BUILD_STAND_ALONE
#include <gtest/gtest.h>
#endif

// 元記事
// https://twitter.com/kuina_ch/status/816977065480069121?lang=ja

namespace {
    // くいなちゃんが出した空白文字一覧(UTF-8)
    // くいなちゃんは17種類挙げたが、サーバで変換されて14種類になってしまったようだ
    // 二番目は0xa0に戻してあるので、計15種類
    const std::string SpaceSet = "「 」「 」「 」「 」「 」「 」「 」「 」「 」「 」「 」「 」「 」「 」「 」「 」「　」";
    // 空白以外は一致しないことを確認する
    const std::string NonSpaceSet = "「.」「,」「x」「!」";
    // 正規表現パターン(POSIX文字クラスの空白:UTF-8)。US-ASCII以外の空白もつかまえる。
    const std::string PosixSpacePattern = "(「[[:space:]]」)";
    // 正規表現パターン(メタ文字)
    const std::string MetaSpacePattern = "(「\\s」)";

    // Unicodeの一文字
    using CodePoint = uint32_t;
    // 出現した文字を登録する表
    using OccurredCodePoint = std::unordered_map<CodePoint, bool>;

    auto Utf8ToUtf16(const std::string& str) {
        return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(str);
    }

    template <typename T>
    auto ConvertToUtf8(const T& str) {
        using CharType = typename T::value_type;
        return std::wstring_convert<std::codecvt_utf8_utf16<CharType>, CharType>{}.to_bytes(str);
    }

    template <typename T>
    void PrintCodePoints(const T& str, std::ostream& os) {
        const auto strInner = Utf8ToUtf16(str);

        for(auto c : strInner) {
            // 「」以外を表示する
            if ((c != 0x300c) && (c != 0x300d)) {
                auto code = static_cast<CodePoint>(c);
                os << std::hex << code << ":";
            }
        }

        return;
    }

    template <typename T, typename U>
    size_t ScanSpaces(const T& str, const U& pattern, std::ostream& os) {
        const auto strInner = Utf8ToUtf16(str);
        const auto patternInner = Utf8ToUtf16(pattern);

        // 正規表現
        boost::basic_regex<wchar_t> expr(patternInner.begin(), patternInner.end());
        // 「」区切りのイテレータ
        boost::regex_token_iterator<decltype(strInner.begin()), wchar_t> i {strInner.begin(), strInner.end(), expr, 1};
        // 「」区切りのイテレータ(終端)
        decltype(i) e;

        // 文字の重複は省く
        OccurredCodePoint codeMap;
        while(i != e) {
            const auto strPart = i->str();
            // 「 」をキャプチャしたはずがそうではなかった
            if (strPart.size() < 3) {
                continue;
            }

            const auto code = static_cast<CodePoint>(strPart.at(1));
            if (codeMap.find(code) == codeMap.end()) {
                const auto s = ConvertToUtf8(strPart);
                // UTF-8にしてから、文字と文字コードを表示する
                os << s << ":" << std::hex << code << "&";
            }

            codeMap[code] = true;
            ++i;
        }

        // 何種類の文字を空白として扱ったか
        auto count = codeMap.size();
        os << std::dec << "\n" << count << " spaces found";
        return count;
    }
}

// このディレクトリ全体のテストを実行する
#ifndef CPPFRIENDS_REGEX_BUILD_STAND_ALONE
class TestSpaceSet : public ::testing::Test{};

TEST_F(TestSpaceSet, PosixClass) {
    // 結果を受け取る文字ストリーム
    std::ostringstream os;
    PrintCodePoints(SpaceSet, os);
    ScanSpaces(SpaceSet, PosixSpacePattern, os);

#if defined(__MINGW32__) || defined(__MINGW64__) || (defined(__GNUC__) && __GNUC__ >= 7)
    // MinGW GCC 7.1.0 の出力結果
    std::string expected = "20:a0:1680:2002:2003:2002:2003:2004:2005:"
        "2006:2007:2008:2009:200a:202f:205f:3000:"
        "「 」:20&「 」:a0&「 」:1680&「 」:2002&「 」:2003&「 」:2004&"
        "「 」:2005&「 」:2006&「 」:2007&「 」:2008&「 」:2009&"
        "「 」:200a&「 」:202f&「 」:205f&「　」:3000&\n15 spaces found";
#else
    // Cygwin GCC 6.3.0 の出力結果
    std::string expected = "20:a0:1680:2002:2003:2002:2003:2004:2005:"
        "2006:2007:2008:2009:200a:202f:205f:3000:"
        "「 」:20&「 」:1680&「 」:2002&「 」:2003&「 」:2004&"
        "「 」:2005&「 」:2006&「 」:2008&「 」:2009&"
        "「 」:200a&「 」:205f&「　」:3000&\n12 spaces found";
#endif
    EXPECT_EQ(expected, os.str());
}

TEST_F(TestSpaceSet, MetaSpace) {
    std::ostringstream os;
#if defined(__MINGW32__) || defined(__MINGW64__) || (defined(__GNUC__) && __GNUC__ >= 7)
    constexpr size_t expected = 15;
#else
    constexpr size_t expected = 12;
#endif
    EXPECT_EQ(expected, ScanSpaces(SpaceSet, MetaSpacePattern, os));
}

TEST_F(TestSpaceSet, NonSpace) {
    std::ostringstream os;
    EXPECT_FALSE(ScanSpaces(NonSpaceSet, PosixSpacePattern, os));
    EXPECT_FALSE(ScanSpaces(NonSpaceSet, MetaSpacePattern, os));
}

// このcppだけで実行するexeを作成するときは、以下のマクロが有効になる
#else

int main(int argc, char* argv[]) {
    std::cout << "Run with Boost C++ Libraries " << (BOOST_VERSION / 100000) << "." << (BOOST_VERSION / 100 % 1000);
    std::cout << "." << (BOOST_VERSION % 100) << "\n";

    // MinGW GCC 6.3.0は、Cygwin GCC 5.4.0と異なり、U+00a0, U+2007. U+202fも空白と扱う
    PrintCodePoints(SpaceSet, std::cout);
    auto patternSet = {PosixSpacePattern, MetaSpacePattern};
    for(auto& pattern : patternSet) {
        ScanSpaces(SpaceSet, pattern, std::cout);
        std::cout << "\n";
    }

    // 空白でなければ一致しないことを確認する
    for(auto& pattern : patternSet) {
        std::ostringstream os;
        auto size = ScanSpaces(NonSpaceSet, pattern, os);
        std::cout << pattern << ":";
        std::cout << ((size == 0) ? "Non-space characters skipped\n" : "A non-space character captured!\n");
    }

    return 0;
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
