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

	IP7_Trace *log_trace = connect_logger();
	reg_current_thread(log_trace, "manager");

	while (!stop_threads.load()) {
		log_trace->P7_INFO(0, TM("Manager start"));
		{
			boost::unique_lock<boost::mutex> locker(csv_lock);
			if (is_new_file(name_of_csv) || actual_data.size() == 0) parse_file(name_of_csv, actual_data);
		}	
		boost::this_thread::sleep_for(boost::chrono::milliseconds(5000));
		log_trace->P7_INFO(0, TM("Manager end"));
	}
	log_trace->P7_INFO(0, TM("Close manager"));
	del_current_thread(log_trace);
}




