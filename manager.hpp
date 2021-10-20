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
#include "dailydata.hpp"
#include "csv_handler.hpp"
#include <vector>
#include <condition_variable>
#include <string>

void manager(std::mutex & m_stop, 
             std::condition_variable & stop_threads, 
             std::string & name_of_csv, 
             boost::mutex & csv_lock, 
             std::vector<daily_data> & actual_data);

#endif /* MANAGER_HPP_ */
