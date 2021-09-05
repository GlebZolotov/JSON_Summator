/*
 * manager.hpp
 *
 *  Created on: 6 сент. 2021 г.
 *      Author: gleb
 */

#ifndef MANAGER_HPP_
#define MANAGER_HPP_

#include "boundedbuffer.hpp"
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>

void manager(bool & running, bounded_buffer<std::string> & ring_buffer, std::vector<bounded_buffer<std::string> *> & manager_buffer);

#endif /* MANAGER_HPP_ */
