/*
 * csv_handler.cpp
 *
 *  Created on: 6 окт. 2021 г.
 *      Author: gleb
 */

#include "csv_handler.hpp"

bool is_new_file(std::string &name_of_path) {
    for(auto const& dir_entry: fs::directory_iterator{(fs::path)name_of_path})
        if (fs::is_regular_file(dir_entry.path()) && (dir_entry.path().filename() == "update.txt")) {
            fs::remove(dir_entry.path());
            return true;
        }
    return false; 
}
