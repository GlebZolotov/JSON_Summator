/*
 * worker_thread.cpp
 *
 *  Created on: 5 сент. 2021 г.
 *      Author: gleb
 */

#include "worker_thread.hpp"

void worker_thread( bool & running, 
                    bounded_buffer< std::pair<true_input_type, bool>* > & manager_buffer, 
                    cppkafka::BufferedProducer<std::string> & producer, 
					std::vector<std::string> output_topics_name,
                    std::string & name_of_csv, 
                    boost::mutex & csv_lock,
                    std::vector<daily_data> & manager_actual_data) {

	std::vector< cppkafka::ConcreteMessageBuilder<std::string> > builders;
	for (std::string cur_name : output_topics_name) builders.emplace_back(cur_name);

	std::string cur_thr_name_csv(name_of_csv);
	std::vector<daily_data> cur_actual_data;
	
	while(running) {
		{
			boost::unique_lock<boost::mutex> locker(csv_lock);
			if (name_of_csv != cur_thr_name_csv || cur_actual_data.size() == 0) cur_actual_data = manager_actual_data;
		}

		std::pair<true_input_type, bool>* new_data;
		std::string res_data;
		// Get message
		manager_buffer.pop_back(&new_data);
		try{
			// De-serialization
			true_input_type new_value = new_data->first;
			INFO << "New message from buffer";

			// Work
			true_output_type res_value = worker(new_value);

			// Serialization
			res_data = serialization(res_value);

			// Send message
			for (int i = 0; i < (int)output_topics_name.size(); i++) {
				if (output_topics_name[i] == new_value.get_topic()) {
					builders[i].payload(res_data);
					producer.produce(builders[i]);
					INFO<< "Message sent to Kafka";
					new_data->second = true;
					break;
				}
			}
		}
		catch(...){
			INFO << "Exception was caught";
		}
	}
}




