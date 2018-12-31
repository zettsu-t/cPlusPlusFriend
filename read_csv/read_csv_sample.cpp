#include <fstream>
#include <iostream>
#include <string>
#include <boost/algorithm/string.hpp>
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