/*
 * worker_thread.cpp
 *
 *  Created on: 5 сент. 2021 г.
 *      Author: gleb
 */

#include "worker_thread.hpp"

void worker_thread(bool & running, bounded_buffer< std::pair<Message&, bool>* > & manager_buffer, cppkafka::BufferedProducer<std::string> & producer, std::string output_topic_name) {
	//BOOST_LOG_SCOPED_THREAD_TAG("ThreadID", boost::this_thread::get_id());
	cppkafka::ConcreteMessageBuilder<std::string> builder(output_topic_name);
	while(running) {
		std::pair<Message&, bool>* new_data;
		std::string res_data;
		// Get message
		manager_buffer.pop_back(&new_data);
		try{
			// De-serialization
			true_input_type new_value = deserialization(new_data->first.get_payload());

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




