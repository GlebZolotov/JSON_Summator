#include "logging.hpp"
#include "boundedbuffer.hpp"
#include "worker_thread.hpp"
#include "manager.hpp"
#include <stdexcept>
#include <iostream>
#include <csignal>
#include <boost/program_options.hpp>
#include <cppkafka/consumer.h>
#include <cppkafka/configuration.h>
#include <boost/thread.hpp>
#include <chrono>


using std::string;
using std::exception;
using std::cout;
using std::endl;

using cppkafka::Consumer;
using cppkafka::Configuration;
using cppkafka::Message;
using cppkafka::TopicPartitionList;
using namespace std::chrono_literals;

namespace po = boost::program_options;

bool running = true;

int main(int argc, char* argv[]) {
    string brokers;
    string input_topic_name;
    string output_topic_name;
    string group_id;
    int N;
    int size_of_buffer;

    po::options_description options("Options");
    options.add_options()
        ("help,h",     			"produce this help message")
        ("brokers,b",  			po::value<string>(&brokers)->required(),
                       	   	   	"the kafka broker list")
        ("input_topic,i",    	po::value<string>(&input_topic_name)->required(),
                       	   	   	"the topic for reading messages")
		("output_topic,o",    	po::value<string>(&output_topic_name)->required(),
								"the topic for writing messages")
        ("group-id,g", 			po::value<string>(&group_id)->required(),
                       	   	    "the consumer group id")
	    ("number_of_workers,n", po::value<int>(&N)->required(),
							  	"count of workers")
		("size_of_buffer,s", 	po::value<int>(&size_of_buffer)->required(),
							  	"size of buffer")
        ;

    po::variables_map vm;

    try {
		po::store(po::command_line_parser(argc, argv).options(options).run(), vm);
		po::notify(vm);
	}
	catch (exception& ex) {
		cout << "Error parsing options: " << ex.what() << endl;
		cout << endl;
		cout << options << endl;
		return 1;
	}

	// Stop processing on SIGINT
	signal(SIGINT, [](int) { running = false; });

	// Construct the configuration
	Configuration config_cons = {
		{ "metadata.broker.list", brokers },
		{ "group.id", group_id },
		// Disable auto commit
		{ "enable.auto.commit", false }
	};

	Configuration config_prod = {
			{ "metadata.broker.list", brokers }
		};

	// Create the consumer
	Consumer consumer(config_cons);
	cppkafka::BufferedProducer<string> producer(config_prod);

	bounded_buffer< std::pair<Message&, bool>* > ring_buffer(size_of_buffer);
	std::vector< bounded_buffer< std::pair<Message&, bool>* > *> manager_buffer;

	// Create threads
	boost::thread_group thrs;
	for(int i = 0; i < N; i++) {
		// Можно и не 1, а побольше
		bounded_buffer< std::pair<Message&, bool>* > *buf = new bounded_buffer< std::pair<Message&, bool>* >(1);
		manager_buffer.push_back(buf);
		boost::thread *t = new boost::thread(worker_thread, std::ref(running), std::ref(*buf), std::ref(producer), output_topic_name);
		thrs.add_thread(t);
	}
	boost::thread manager_t{manager, std::ref(running), std::ref(ring_buffer), std::ref(manager_buffer)};

	// Subscribe to the topic
	consumer.subscribe({ input_topic_name });

	std::list< std::pair<Message&, bool>* > list_of_messages;
	std::list< std::vector<Message> > msgs;

	// Now read lines and write them into kafka
	while (running) {
		// Try to consume a message
		if (ring_buffer.is_need_inc()) {
			msgs.push_front(consumer.poll_batch(size_of_buffer / 2));
			int new_size(msgs.front().size());
			//msgs.insert(msgs.begin(), new_msgs.begin(), new_msgs.end());
			while (new_size > 0) {
				std::pair<Message&, bool>* msg = new std::pair<Message&, bool>(msgs.front()[new_size - 1], false);
				new_size--;
				if (msg->first and !(msg->first.get_error())) {
					// If we managed to get a message
					ring_buffer.push_front(msg);
					list_of_messages.push_back(msg);
					INFO << "New message from Kafka";
				}
				else msgs.front().erase(msgs.front().begin()+new_size);
			}
		}

		int end_of_handled(0);
		std::pair<Message&, bool>* prev_msg = nullptr;
		for (std::list< std::pair<Message&, bool>* >::iterator i=list_of_messages.begin(); i!=list_of_messages.end(); i++) {
			if ((*i)->second) {
				if (prev_msg) {
					while (!msgs.empty() && msgs.back().empty()) msgs.pop_back();
					if (!msgs.empty()) msgs.back().pop_back();
				}
				prev_msg = *i;
				end_of_handled++;
			}
			else break;
		}
		// commit last handled message
		if (prev_msg) {
			consumer.commit(prev_msg->first);
			while (!msgs.empty() && msgs.back().empty()) msgs.pop_back();
			if (!msgs.empty()) msgs.back().pop_back();
		}
		if (end_of_handled > 0) {
			std::list< std::pair<Message&, bool>* >::iterator i1=list_of_messages.begin();
			std::list< std::pair<Message&, bool>* >::iterator i2=list_of_messages.begin();
			// clean list
			std::advance(i2, end_of_handled);
			list_of_messages.erase(i1, i2);
		}

	}
	manager_t.interrupt();
	thrs.interrupt_all();
	for(int i=0; i<manager_buffer.size(); i++) delete manager_buffer[i];
}
