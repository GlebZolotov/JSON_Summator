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
                    std::string output_topic_name, 
                    std::string & name_of_csv, 
                    boost::mutex & csv_lock) {

	cppkafka::ConcreteMessageBuilder<std::string> builder(output_topic_name);
	std::string cur_thr_name_csv(name_of_csv);
	vector<daily_data> actual_data;
	
	while(running) {
		std::pair<true_input_type, bool>* new_data;
		std::string res_data;
		// Get message
		manager_buffer.pop_back(&new_data);
		try{
			// De-serialization
			true_input_type new_value = new_data->first;

			INFO << "New message from buffer";

			// Validation
			if(validation(new_value)) {

				// Work
				true_output_type res_value = worker(new_value);

				// Serialization
				res_data = serialization(res_value);

				// Send message
				builder.payload(res_data);
				producer.produce(builder);
				INFO<< "Message sent to Kafka";
			} else {
				INFO << "Broken message (not valid)";
			}
			new_data->second = true;
		}
		catch(...){
			INFO << "Exception was caught";
		}
	}
}




