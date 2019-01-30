// [[Rcpp::depends(BH)]]
#include <algorithm>
#include <string>
#include <unordered_map>
#include <Rcpp.h>

namespace {
    using StringTable = std::unordered_map<std::string, std::string>;
    StringTable g_table;
}

// [[Rcpp::export]]
Rcpp::NumericVector clear_table(Rcpp::NumericVector dummy) {
    g_table.clear();
    return 0;
}

// [[Rcpp::export]]
Rcpp::NumericVector add_table(Rcpp::CharacterVector keys, Rcpp::CharacterVector values) {
    auto size = std::min(keys.size(), values.size());
    for(decltype(size) i=0; i<size; ++i) {
        auto k = Rcpp::as<decltype(g_table)::key_type>(keys.at(i));
        auto v = Rcpp::as<decltype(g_table)::mapped_type>(values.at(i));
        g_table[k] = v;
    }
    return 0;
}

// [[Rcpp::export]]
Rcpp::CharacterVector find_table(Rcpp::CharacterVector keys) {
    Rcpp::CharacterVector results;
    for(const auto& key : keys) {
        auto k = Rcpp::as<decltype(g_table)::key_type>(key);
        if (g_table.count(k)) {
            results.push_back(g_table[k]);
        } else {
            results.push_back("");
        }
    }
    return results;
}

// [[Rcpp::export]]
Rcpp::DataFrame convert_table_to_dataframe(Rcpp::NumericVector dummy) {
    const auto rowSize = g_table.size();
    Rcpp::CharacterVector keys(rowSize);
    Rcpp::CharacterVector values(rowSize);

    size_t i = 0;
    for (const auto& keyValue : g_table) {
        const auto key = keyValue.first;
        keys[i] = key;
        ++i;
    }

    keys.sort();
    i = 0;
    for (const auto& key : keys) {
        values[i] = g_table[Rcpp::as<decltype(g_table)::key_type>(key)];
        ++i;
    }

    Rcpp::DataFrame df = Rcpp::DataFrame::create(
        Rcpp::Named("key")=keys, Rcpp::Named("value")=values);
    return df;
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
