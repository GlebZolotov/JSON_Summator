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

std::vector<uint8_t> worker(std::vector<uint8_t> & input_message);

#endif /* WORKER_HPP_ */
