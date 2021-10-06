/*
 * worker_thread.hpp
 *
 *  Created on: 4 сент. 2021 г.
 *      Author: gleb
 */

#ifndef WORKER_THREAD_HPP_
#define WORKER_THREAD_HPP_

#include "worker.hpp"
#include "logging.hpp"
#include "boundedbuffer.hpp"
#include <cppkafka/utils/buffered_producer.h>

void worker_thread( bool & running, 
                    bounded_buffer< std::pair<true_input_type, bool>* > & manager_buffer, 
                    cppkafka::BufferedProducer<std::string> & producer, 
                    std::string output_topic_name, 
                    std::string & name_of_csv, 
                    boost::mutex & csv_lock
                    );

#endif /* WORKER_THREAD_HPP_ */
