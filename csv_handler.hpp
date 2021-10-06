/*
 * csv_handler.hpp
 *
 *  Created on: 6 окт. 2021 г.
 *      Author: gleb
 */

#ifndef CSV_HANDLER_HPP_
#define CSV_HANDLER_HPP_

#include <vector>
#include <string>
#include <filesystem>
#include "dailydata.hpp"
#include <iostream>
#include <fstream>

using std::vector;
using std::string;
using std::ifstream;

namespace fs = std::filesystem;

bool is_new_file(string &name_of_file);
bool parse_file(string &name_of_file, vector<daily_data> & res);


#endif /* CSV_HANDLER_HPP_ */
