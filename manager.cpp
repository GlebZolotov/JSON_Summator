/*
 * manager.cpp
 *
 *  Created on: 6 сент. 2021 г.
 *      Author: gleb
 */

#include "manager.hpp"

void manager(std::atomic<bool> & stop_threads,
			 std::string & name_of_csv, 
			 boost::mutex & csv_lock, 
			 std::vector<daily_data> & actual_data) {
	while (!stop_threads.load()) {
		INFO << "Manager start";
		{
			boost::unique_lock<boost::mutex> locker(csv_lock);
			if (is_new_file(name_of_csv) || actual_data.size() == 0) parse_file(name_of_csv, actual_data);
		}	
		boost::this_thread::sleep_for(boost::chrono::milliseconds(5000));
		INFO << "Manager end";
	}
	INFO << "Close manager";
}




