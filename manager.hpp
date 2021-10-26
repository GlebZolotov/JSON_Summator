/*
 * manager.hpp
 *
 *  Created on: 6 сент. 2021 г.
 *      Author: gleb
 */

#ifndef MANAGER_HPP_
#define MANAGER_HPP_

#include "boundedbuffer.hpp"
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include "csv_handler.hpp"
#include <vector>
#include <string>
#include "logging.hpp"
#include "worker.hpp"
#include "rapidcsv.h"

void manager(std::atomic<bool> & stop_threads, 
             std::string & name_of_csv, 
             boost::mutex & csv_lock, 
             std::vector<rapidcsv::Document> & actual_data);

#endif /* MANAGER_HPP_ */
