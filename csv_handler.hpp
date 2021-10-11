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

namespace fs = std::filesystem;

bool is_new_file(std::string &name_of_file);
void parse_file(std::string &name_of_file, std::vector<daily_data> & res);


#endif /* CSV_HANDLER_HPP_ */
