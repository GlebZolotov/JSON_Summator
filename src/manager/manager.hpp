/*
 * manager.hpp
 *
 *  Created on: 6 сент. 2021 г.
 *      Author: gleb
 */

#pragma once

#include "boundedbuffer.hpp"
#include <chrono>
#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include "logging.hpp"
#include "worker.hpp"
#include "rapidcsv.h"

namespace fs = std::filesystem;

bool is_new_file(std::string &name_of_path);

void manager(std::atomic<bool> & stop_threads, 
             std::string & name_of_csv, 
             std::mutex & csv_lock, 
             std::vector<rapidcsv::Document> & actual_data);
