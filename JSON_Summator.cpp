#include "boundedbuffer.hpp"
#include <stdexcept>
#include <iostream>
#include <csignal>
#include <boost/program_options.hpp>
#include <cppkafka/consumer.h>
#include <cppkafka/configuration.h>


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
    string topic_name;
    string group_id;
    int N;
    int size_of_buffer;

    po::options_description options("Options");
    options.add_options()
        ("help,h",     			"produce this help message")
        ("brokers,b",  			po::value<string>(&brokers)->required(),
                       	   	   	"the kafka broker list")
        ("topic,t",    			po::value<string>(&topic_name)->required(),
                       	   	   	"the topic in which to write to")
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

	bounded_buffer< std::vector<uint8_t> > ring_buffer(size_of_buffer);

	// Create threads


	// Stop processing on SIGINT
	signal(SIGINT, [](int) { running = false; });

	// Construct the configuration
	Configuration config = {
		{ "metadata.broker.list", brokers },
		{ "group.id", group_id },
		// Disable auto commit
		{ "enable.auto.commit", false }
	};

	// Create the consumer
	Consumer consumer(config);

	// Print the assigned partitions on assignment
	consumer.set_assignment_callback([](const TopicPartitionList& partitions) {
		cout << "Got assigned: " << partitions << endl;
	});

	// Print the revoked partitions on revocation
	consumer.set_revocation_callback([](const TopicPartitionList& partitions) {
		cout << "Got revoked: " << partitions << endl;
	});

	// Subscribe to the topic
	consumer.subscribe({ topic_name });

	cout << "Consuming messages from topic " << topic_name << endl;

	std::vector <uint8_t> new_msg;

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

			}
		}
	}
}
/*
int main(int argc, char **argv) {
	ptree doc;




	istringstream obuf(string(output_message.begin(), output_message.end()));
	read_json(obuf, doc);

	cout << doc.get<std::string>("rqId") << endl;
	cout << doc.get<std::string>("model") << endl;
	cout << doc.get<long long int>("result") << endl;
	return 0;
}*/
