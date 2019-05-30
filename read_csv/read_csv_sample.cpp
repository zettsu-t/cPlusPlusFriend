#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/tokenizer.hpp>
#include <gtest/gtest.h>

class CsvFile {
public:
    using CellSet = std::vector<std::string>;

    CsvFile(const std::string& filename) {
        auto ifs = std::ifstream(filename);
        parse(ifs);
    }

    virtual ~CsvFile(void) = default;

    const CellSet& GetCells(void) {
        return cells_;
    }

private:
    void parse(std::ifstream& ifs) {
        using namespace boost;
        std::string line;

        // Assumes no items contain LF
        while(std::getline(ifs, line)) {
            const auto lineSize = line.size();
            // Removes a trailing CR if it exists
            if (lineSize > 0 && line.at(lineSize - 1) == '\r') {
                line.resize(lineSize - 1);
            }

            if (line.empty()) {
                continue;
            }

            tokenizer<escaped_list_separator<char>> t(
                line, escaped_list_separator<char>('\\', ',', '\"'));
            for (tokenizer<escaped_list_separator<char>>::iterator i(t.begin());
                 i != t.end(); ++i) {
                cells_.push_back(boost::trim_copy(*i));
            }
            cells_.push_back("\n");
        }

        return;
    }

    CellSet cells_;
};

class TestCsvFile : public ::testing::Test{
protected:
    void Compare(const CsvFile::CellSet& expected, const CsvFile::CellSet& actual) {
        const auto cellSize = expected.size();
        ASSERT_TRUE(cellSize);
        ASSERT_EQ(cellSize, actual.size());

        for(auto i = static_cast<decltype(cellSize)>(0); i < cellSize; ++i) {
            EXPECT_EQ(expected.at(i), actual.at(i));
        }
    }
};

TEST_F(TestCsvFile, CRLF) {
    const std::vector<std::string> expected {
        "Cell1",  "Cell2",  "Cell3",  "\n",
        "Cell 4", "Cell 5", "Cell 6", "\n",
        "Cell 7", "Cell 8", "Cell 9", "\n"
    };

    CsvFile csvfile("crlf.csv");
    const auto& actual = csvfile.GetCells();
    Compare(expected, actual);
}

TEST_F(TestCsvFile, LF) {
    const std::vector<std::string> expected {
        "1.2", "3.4", "5.6", "\n",
        "2.1", "4.3", "6.5", "\n",
        "1.2.3", "4.5.6", "7.8.9", "\n"
    };

    CsvFile csvfile("lf.csv");
    const auto& actual = csvfile.GetCells();
    Compare(expected, actual);
}

namespace {
    int lineToInt(const std::string& line) {
        return boost::lexical_cast<int>(line);
    }
}

class TestTrailingLf : public ::testing::Test{};

TEST_F(TestTrailingLf, ToInt) {
    constexpr int expected = 123;
    auto ifs = std::ifstream("crlfnums.csv");
    std::string line;
    std::getline(ifs, line);
    ASSERT_FALSE(line.empty());

#if defined(__MINGW32__) || defined(__MINGW64__)
    ASSERT_EQ(expected, lineToInt(line));
#else
    ASSERT_ANY_THROW(lineToInt(line));
#endif

    boost::trim(line);
    ASSERT_FALSE(line.empty());
    ASSERT_EQ(expected, lineToInt(line));
}

TEST_F(TestTrailingLf, CRLF) {
    constexpr char expected = '3';
    auto ifs = std::ifstream("crlf.csv");
    std::string line;
    std::getline(ifs, line);

    ASSERT_FALSE(line.empty());
#if defined(__MINGW32__) || defined(__MINGW64__)
    ASSERT_EQ(expected, *(line.rbegin()));
#else
    ASSERT_EQ('\r', *(line.rbegin()));
#endif

    boost::trim(line);
    ASSERT_FALSE(line.empty());
    ASSERT_EQ(expected, *(line.rbegin()));
}

TEST_F(TestTrailingLf, LF) {
    auto ifs = std::ifstream("lf.csv");
    std::string line;
    std::getline(ifs, line);
    std::getline(ifs, line);

    ASSERT_FALSE(line.empty());
    ASSERT_EQ('6', *line.rbegin());

    boost::trim(line);
    ASSERT_FALSE(line.empty());
    ASSERT_EQ('6', *line.rbegin());
}

template <typename T>
class CsvParser {
public:
    using Setter = void(T::*)(const std::string& value);
    using ParamToSetter = std::unordered_map<std::string, Setter>;
    CsvParser(std::istream& is, const ParamToSetter& paramToSetter) {
        using namespace boost;
        std::string line;
        std::getline(is, line);

        tokenizer<escaped_list_separator<char>> t(
            line, escaped_list_separator<char>('\\', ',', '\"'));

        ColumnNumber columnNumber = 0;
        for (tokenizer<escaped_list_separator<char>>::iterator i(t.begin());
             i != t.end(); ++i) {
            auto columnName = *i;
            auto it = paramToSetter.find(columnName);
            if (it != paramToSetter.end()) {
                columnSetter_[columnNumber] = it->second;
            }
            ++columnNumber;
        }
    }

    virtual ~CsvParser(void) = default;

    boost::optional<T> ParseRow(std::istream& is) {
        using namespace boost;
        std::string line;
        std::getline(is, line);
        boost::trim_copy(line);

        if (line.empty()) {
            boost::optional<T> emptyRow;
            return emptyRow;
        }

        T row;
        tokenizer<escaped_list_separator<char>> t(
            line, escaped_list_separator<char>('\\', ',', '\"'));

        ColumnNumber columnNumber = 0;
        for (tokenizer<escaped_list_separator<char>>::iterator i(t.begin());
             i != t.end(); ++i) {
            auto it = columnSetter_.find(columnNumber);
            if (it != columnSetter_.end()) {
                (row.*(it->second))(*i);
            }
            ++columnNumber;
        }

        return row;
    }
private:
    using ColumnNumber = typename ParamToSetter::size_type;
    using ColumnSetter = std::unordered_map<ColumnNumber, Setter>;
    ColumnSetter columnSetter_;
};

class TestCsvParser : public ::testing::Test{};

TEST_F(TestCsvParser, All) {
    struct Param {
        std::string name;
        int index {0};
        void SetName(const std::string& value) {
            name = value;
        }
        void SetIndex(const std::string& value) {
            index = boost::lexical_cast<decltype(index)>(value);
        }
    };

    using Parser = CsvParser<Param>;
    Parser::ParamToSetter setter {
        {"name", &Param::SetName},
        {"index", &Param::SetIndex}
    };

    std::istringstream is("name,index\nfoo,1\n");
    Parser parser(is, setter);

    auto row = parser.ParseRow(is);
    ASSERT_TRUE(row);
    EXPECT_EQ("foo", row->name);
    EXPECT_EQ(1, row->index);
    ASSERT_FALSE(parser.ParseRow(is));

    std::istringstream isNext("bar,22\n");
    row = parser.ParseRow(isNext);
    ASSERT_TRUE(row);
    EXPECT_EQ("bar", row->name);
    EXPECT_EQ(22, row->index);
}

int main(int argc, char* argv[]) {
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
