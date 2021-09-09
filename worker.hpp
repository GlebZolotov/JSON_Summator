/*
 * worker.hpp
 *
 *  Created on: 4 сент. 2021 г.
 *      Author: gleb
 */

#ifndef WORKER_HPP_
#define WORKER_HPP_

#include <boost/archive/basic_archive.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

typedef boost::property_tree::ptree true_input_type;
typedef boost::property_tree::ptree true_output_type;

true_output_type worker(true_input_type & input_tree);
true_input_type deserialization(const std::string & input_message);
std::string serialization(true_output_type & output_tree);
bool validation(true_input_type & input_tree);

#endif /* WORKER_HPP_ */
