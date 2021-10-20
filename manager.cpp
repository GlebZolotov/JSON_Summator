/*
 * manager.cpp
 *
 *  Created on: 6 сент. 2021 г.
 *      Author: gleb
 */

#include "manager.hpp"

void manager(std::mutex & m_stop, 
			 std::condition_variable & stop_threads, 
			 std::string & name_of_csv, 
			 boost::mutex & csv_lock, 
			 std::vector<daily_data> & actual_data) {
	while (true) {
		{
			boost::unique_lock<boost::mutex> locker(csv_lock);
			if (is_new_file(name_of_csv) || actual_data.size() == 0) parse_file(name_of_csv, actual_data);
		}	
		boost::this_thread::sleep_for(boost::chrono::milliseconds(5000));

		//std::unique_lock<std::mutex> l(m_stop);
        //if (stop_threads.wait_for(l, std::chrono::milliseconds(10)) == std::cv_status::no_timeout) break;
	}
}




