/*
 * worker.hpp
 *
 *  Created on: 4 сент. 2021 г.
 *      Author: gleb
 */

#ifndef WORKER_HPP_
#define WORKER_HPP_

#include <google/protobuf/util/json_util.h>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <thread>
#include "input_data.pb.h"
#include "output_data.pb.h"
#include "library/test_solver.hpp"
#include "logging.hpp"


typedef input_proto::InputData true_input_type;
typedef output_proto::OutputData true_output_type;

true_output_type worker(true_input_type & input_tree);
true_input_type deserialization(const std::string & input_message);
MadStatement & construct_input_for_solver(const true_input_type & in_m, MadStatement & in_s);
std::string serialization(true_output_type & output_tree);
true_output_type & construct_output_from_solver(true_output_type & out_m, const true_input_type & in_m, const MadSolution & out_s);
bool validation(true_input_type & input_tree);

#endif /* WORKER_HPP_ */
