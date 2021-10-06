/*
 * csv_handler.cpp
 *
 *  Created on: 6 окт. 2021 г.
 *      Author: gleb
 */

#include "csv_handler.hpp"

bool is_new_file(string &name_of_file) {
    bool res(fs::exists(name_of_file));
    if (!res) {
        for(auto const& dir_entry: fs::directory_iterator{((fs::path)name_of_file).parent_path()})
            if (fs::is_regular_file(dir_entry.path()) && (dir_entry.path().extension() == ".csv")) {
                name_of_file = dir_entry.path().u8string();
                break;
            }
    }
    return !res; 
}

bool parse_file(string &name_of_file, vector<daily_data> & res) {
    ifstream in_file(name_of_file);
    string cur_str;
    bool skip_header(true);

    res.clear();
    while(std::getline(in_file, cur_str)) {
        if (skip_header) {
            skip_header = false;
            continue;
        }
        daily_data cur_data;
        std::stringstream lineStream(cur_str);
        string cell;
        vector<string> result;

        while(std::getline(lineStream, cell, ';')) result.push_back(cell);
        if (result.size() >= 3) res.emplace_back(result[0], atof(result[1].c_str()), (bool)atoi(result[2].c_str()));
    }
}