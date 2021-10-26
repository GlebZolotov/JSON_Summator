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
#include <iostream>
#include <fstream>

namespace fs = std::filesystem;

bool is_new_file(std::string &name_of_path);


#endif /* CSV_HANDLER_HPP_ */
