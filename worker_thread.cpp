/*
 * worker_thread.cpp
 *
 *  Created on: 5 сент. 2021 г.
 *      Author: gleb
 */

#include "worker_thread.hpp"

void worker_thread(bool & running, bounded_buffer<std::string> & manager_buffer, cppkafka::BufferedProducer<std::string> & producer, std::string output_topic_name) {
	//BOOST_LOG_SCOPED_THREAD_TAG("ThreadID", boost::this_thread::get_id());
	cppkafka::ConcreteMessageBuilder<std::string> builder(output_topic_name);
	while(running) {
		std::string new_data;
		// Get message
		manager_buffer.pop_back(&new_data);
		try{
			// De-serialization
			true_input_type new_value = deserialization(new_data);

			INFO << "New message from buffer";

			// Validation
			if(validation(new_value)) {

				// Work
				true_output_type res_value = worker(new_value);

				// Serialization
				new_data = serialization(res_value);

				// Send message
				builder.payload(new_data);
				producer.produce(builder);
				INFO<< "Message sent to Kafka";
			} else {
				INFO << "Broken message";
			}
		}
		catch(...){
			INFO << "Exception was caught";
		}
	}
}




