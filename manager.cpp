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
			 std::vector<rapidcsv::Document> & actual_data) {

	IP7_Trace *log_trace = connect_logger();
	reg_current_thread(log_trace, "manager");
	std::string cur_name_of_csv(name_of_csv);

	while (!stop_threads.load()) {
		log_trace->P7_INFO(0, TM("Manager start"));
		{
			boost::unique_lock<boost::mutex> locker(csv_lock);
			if (is_new_file(cur_name_of_csv)) {
				actual_data[RETURNS_IND] = rapidcsv::Document(name_of_csv + std::string("/returns.csv"), rapidcsv::LabelParams(0, 0));
    			actual_data[META_IND] = rapidcsv::Document(name_of_csv + std::string("/meta.csv"), rapidcsv::LabelParams(0, 0));
    			actual_data[CLOSE_IND] = rapidcsv::Document(name_of_csv + std::string("/close.csv"), rapidcsv::LabelParams(0, 0));
				name_of_csv = name_of_csv + "1";
				log_trace->P7_INFO(0, TM("Update files"));
			}
		}	
		boost::this_thread::sleep_for(boost::chrono::milliseconds(5000));
		log_trace->P7_INFO(0, TM("Manager end"));
	}
	log_trace->P7_INFO(0, TM("Close manager"));
	del_current_thread(log_trace);
}




