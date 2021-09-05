#include "logging.hpp"
#include "boundedbuffer.hpp"
#include "worker_thread.hpp"
#include <stdexcept>
#include <iostream>
#include <csignal>
#include <boost/program_options.hpp>
#include <cppkafka/consumer.h>
#include <cppkafka/configuration.h>
#include <boost/thread.hpp>

using std::string;
using std::exception;
using std::cout;
using std::endl;

using cppkafka::Consumer;
using cppkafka::Configuration;
using cppkafka::Message;
using cppkafka::TopicPartitionList;

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

	bounded_buffer< std::string > ring_buffer(size_of_buffer);
	bounded_buffer< std::string > manager_buffer(1);

	// Create threads
	boost::thread_group thrs;
	for(int i = 0; i < N; i++) {
		boost::thread *t = new boost::thread(worker_thread, std::ref(running), std::ref(ring_buffer), std::ref(producer), output_topic_name);
		thrs.add_thread(t);
	}
	//BOOST_LOG_SCOPED_THREAD_TAG("ThreadID", boost::this_thread::get_id());

	// Subscribe to the topic
	consumer.subscribe({ input_topic_name });

	std::string new_msg;

	// Now read lines and write them into kafka
	while (running) {
		// Try to consume a message
		Message msg = consumer.poll();
		if (msg) {
			// If we managed to get a message
			if (!msg.get_error()) {
				new_msg = msg.get_payload();
				ring_buffer.push_front(new_msg);
				// Now commit the message
				consumer.commit(msg);
				INFO << "New message from Kafka";
			}
		}
	}
	thrs.interrupt_all();
}
