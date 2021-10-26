// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#include <fstream>

#include "utils.hpp"

#include "csv.hpp"

namespace mad_min
{

std::tuple<d_matrix /* retG */, d_cvector /* gran */>
load_csv(const std::string& csv_path, const char delimeter)
{
    std::ifstream csv(csv_path);

    std::string line;
    std::vector<std::string> lines;
    while (csv.good() && !csv.eof()) {
        std::getline(csv, line);
        trim(line);

        if (!line.empty()) {
            lines.push_back(line);
        }
    }
    assert(lines.size() > 0);

    auto c = std::count(lines.front().begin(), lines.front().end(), delimeter);
    assert(c > 2);

    const uint32_t T = c - 2;
    const uint32_t W = lines.size() - 1;

    d_cvector gran(W);
    d_matrix retG(T, W);

    for (uint32_t w = 0; w < W; ++w) {
        size_t pos = 0;

        auto token = [&pos, &lines, delimeter, w]() {
            const auto& s = lines[w + 1];

            if (pos == std::string::npos) {
                throw std::runtime_error("");
            }

            auto old_pos = pos;
            pos = s.find(delimeter, pos);
            if (pos == std::string::npos) {
                return trim_copy(s.substr(old_pos));
            }

            pos += 1;

            return trim_copy(s.substr(old_pos, pos - 1 - old_pos));
        };

    #ifndef NDEBUG
        auto index = std::stoi(token());
        assert(index >= 0);
        assert((uint32_t)index == w);
    #endif

        gran[w] = std::stod(token());

        for (uint32_t t = 0; t < T; ++t) {
            retG(t, w) = std::stod(token());
        }

        auto name = token();
        assert(!name.empty());
    }

    return { retG, gran };
}

}
