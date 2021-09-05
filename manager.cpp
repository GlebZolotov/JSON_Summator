/*
 * manager.cpp
 *
 *  Created on: 6 сент. 2021 г.
 *      Author: gleb
 */

#include "manager.hpp"

void manager(bool & running, bounded_buffer<std::string> & ring_buffer, std::vector<bounded_buffer<std::string> *> & manager_buffer) {
	std::string msg;
	while(running) {
		ring_buffer.pop_back(&msg);
		// Вот это надо сделать через cond, чтобы не ждать
		bool q = true;
		while(q) {
			for(int i=0; i<manager_buffer.size(); i++)
				if((*manager_buffer[i]).is_not_full()) {
					(*manager_buffer[i]).push_front(msg);
					q = false;
					break;
				}
			if(q) boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
		}
	}
}




