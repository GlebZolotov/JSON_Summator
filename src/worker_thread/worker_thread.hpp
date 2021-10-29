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
#include <vector>
#include <string>
#include <mutex>
#include <cppkafka/utils/buffered_producer.h>
#include "rapidcsv.h"
#include "loc_utils.hpp"

void worker_thread( std::atomic<bool> & stop_threads,
                    std::vector< bounded_buffer< std::pair<true_input_type, std::atomic<bool> & >* >* > & ring_buffers, 
                    cppkafka::BufferedProducer<std::string> & producer, 
                    std::vector<std::string> output_topics_name,
                    std::string & name_of_csv, 
                    std::mutex & csv_lock,
                    std::vector<rapidcsv::Document> & manager_actual_data,
                    std::string name
                    );

#endif /* WORKER_THREAD_HPP_ */
