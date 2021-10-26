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
			boost::unique_lock<boost::mutex> locker(csv_lock);
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
				// Create solver object and run it into another thread
				log_trace->P7_INFO(0, TM("New message from buffer"));

				MadStatement task;
				std::map<std::string, int> stock_size;
				construct_input_for_solver(new_data->first, task, cur_actual_data, stock_size);
				log_trace->P7_INFO(0, TM("Task ready"));
				ProblemType m_Problem;
				const std::string& model = new_data->first.model();
				if(model == "mad_max")
					m_Problem = ProblemType::MAD_MAX;
				if(model == "mad_min")
					m_Problem = ProblemType::MAD_MIN;
				if(model == "mad_max_lp")
					m_Problem = ProblemType::MAD_MAX_LP;
				if(model == "mad_min_lp")
					m_Problem = ProblemType::MAD_MIN_LP;

				SimplexMethod linearSolver(1e-16);
				LpSolver lpSolver(&linearSolver);
				Solver<MadSolution>* solver;
				log_trace->P7_INFO(0, TM("New solver"));
				switch(m_Problem) {
					case ProblemType::MAD_MAX  :
						solver = new MADMax_Solver(&task);
						break; 
					case ProblemType::MAD_MIN  :
						solver = new MADMin_Solver(&task);
						break; 
					case ProblemType::MAD_MAX_LP  :
						solver = new MadMaxLpSolver(&task, &lpSolver);
						break; 
					case ProblemType::MAD_MIN_LP  :
						solver = new MadMinLpSolver(&task, &lpSolver);
						break; 
					default : 
						std::clog << "Error on problem type\n";
						return;
				}
				
				log_trace->P7_INFO(0, TM("Start solve thread"));
				boost::thread solve_t(&Solver<MadSolution>::Solve, solver);
				// Check status of solving
				MadSolution res;
				do {
					std::this_thread::sleep_for(std::chrono::milliseconds(1000));
					res = solver->Current();
					log_trace->P7_INFO(0, TM("I'm wait..."));
				} while (res.status == SolutionStatus::STARTED || res.status == SolutionStatus::NOT_STARTED);
				log_trace->P7_INFO(0, TM("End solve thread"));
				res = solver->Current();
				// Serialization & Send message
				true_output_type res_out;
				builders[index_of_topic].payload(serialization(construct_output_from_solver(res_out, new_data->first, res, stock_size)));
				producer.produce(builders[index_of_topic]);
				log_trace->P7_INFO(0, TM("Message sent to Kafka"));
				new_data->second.store(true);
				delete solver;
			}
			catch(const std::exception &exc){
				log_trace->P7_INFO(0, TM(exc.what()));
			}
		}
	}
	log_trace->P7_INFO(0, TM("Close thread"));
	del_current_thread(log_trace);
}




