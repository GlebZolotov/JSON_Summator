/*
 * worker_thread.cpp
 *
 *  Created on: 5 сент. 2021 г.
 *      Author: gleb
 */

#include "worker_thread.hpp"

void worker_thread( std::atomic<bool> & stop_threads, 
                    std::vector< bounded_buffer< std::pair<true_input_type, std::atomic<bool> & >* >* > & ring_buffers, 
                    cppkafka::BufferedProducer<std::string> & producer, 
					std::vector<std::string> output_topics_name,
                    std::string & name_of_csv, 
                    std::mutex & csv_lock,
                    std::vector<rapidcsv::Document> & manager_actual_data,
					std::string name) {

	IP7_Trace *log_trace = connect_logger();
	reg_current_thread(log_trace, name.c_str());

	// Create builders for sending messages
	std::vector< cppkafka::ConcreteMessageBuilder<std::string> > builders;
	for (std::string cur_name : output_topics_name) builders.emplace_back(cur_name);

	// Create variable for data from csv
	std::string cur_thr_name_csv(name_of_csv);
	std::vector<rapidcsv::Document> cur_actual_data;
	
	while (!stop_threads.load()) {
		{
			// Check if have new data from csv
			std::unique_lock<std::mutex> locker(csv_lock);
			if (name_of_csv != cur_thr_name_csv || cur_actual_data.size() == 0) {
				cur_actual_data = manager_actual_data;
				cur_thr_name_csv = name_of_csv;
				log_trace->P7_INFO(0, TM("Get csv data"));
			}
		}

		std::pair<true_input_type, std::atomic<bool> & >* new_data;
		std::string res_data;
		// Get message
		for(unsigned int index_of_topic = 0; index_of_topic < ring_buffers.size(); index_of_topic++) {
			if (ring_buffers[index_of_topic]->safe_is_empty()) continue;
			ring_buffers[index_of_topic]->pop_back(&new_data);
			try{
				log_trace->P7_INFO(0, TM("New message from buffer"));

				// Create solver object and run it into another thread
				true_output_type res_output;
				worker(new_data->first, cur_actual_data, log_trace, res_output);

				// Serialization & Send message
				res_data = serialization(res_output);
				write_file(std::string("output_jsons/") + new_data->first.rqid() + std::string(".json"), res_data);
				builders[index_of_topic].payload(res_data);
				producer.produce(builders[index_of_topic]);
				log_trace->P7_INFO(0, TM("Message sent to Kafka"));
				
				// Push signal to commit in main thread
				new_data->second.store(true);
			}
			catch(const std::exception &exc){
				log_trace->P7_INFO(0, TM(exc.what()));
			}
		}
	}
	log_trace->P7_INFO(0, TM("Close thread"));
	del_current_thread(log_trace);
}




