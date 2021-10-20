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
                    boost::mutex & csv_lock,
                    std::vector<daily_data> & manager_actual_data) {

	// Create builders for sending messages
	std::vector< cppkafka::ConcreteMessageBuilder<std::string> > builders;
	for (std::string cur_name : output_topics_name) builders.emplace_back(cur_name);

	// Create variable for data from csv
	std::string cur_thr_name_csv(name_of_csv);
	std::vector<daily_data> cur_actual_data;
	
	while (!stop_threads.load()) {
		{
			// Check if have new data from csv
			boost::unique_lock<boost::mutex> locker(csv_lock);
			if (name_of_csv != cur_thr_name_csv || cur_actual_data.size() == 0) cur_actual_data = manager_actual_data;
		}

		std::pair<true_input_type, std::atomic<bool> & >* new_data;
		std::string res_data;
		// Get message
		for(int index_of_topic = 0; index_of_topic < ring_buffers.size(); index_of_topic++) {
			if (ring_buffers[index_of_topic]->safe_is_empty()) continue;
			ring_buffers[index_of_topic]->pop_back(&new_data);
			try{
				// Create solver object and run it into another thread
				INFO << "New message from buffer";
				MadStatement task;
				TestSolver solveEngine(construct_input_for_solver(new_data->first, task));
				boost::thread solve_t(&TestSolver::Solve, &solveEngine);
				// Check status of solving
				MadSolution res;
				do {
					std::this_thread::sleep_for(std::chrono::milliseconds(1000));
					res = solveEngine.Current();
					INFO << "I'm wait...";
				} while (res.status == SolutionStatus::STARTED);

				// Serialization & Send message
				true_output_type res_out;
				builders[index_of_topic].payload(serialization(construct_output_from_solver(res_out, new_data->first, res)));
				producer.produce(builders[index_of_topic]);
				INFO<< "Message sent to Kafka";
				new_data->second.store(true);
			}
			catch(...){
				INFO << "Exception was caught";
			}
		}
	}
	INFO << "Close thread";
}




