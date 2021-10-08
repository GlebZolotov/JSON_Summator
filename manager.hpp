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

void manager(bool & running, string & name_of_csv, boost::mutex & csv_lock, vector<daily_data> & actual_data);

#endif /* MANAGER_HPP_ */
