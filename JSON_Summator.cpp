#include "logging.hpp"
#include "boundedbuffer.hpp"
#include "worker_thread.hpp"
#include "worker.hpp"
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
using std::vector;
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
    vector<string> input_topic_names;
    vector<string> output_topic_names;
    vector<string> group_ids;
    int N;
    int size_of_buffer;
	int size_of_batch;
	int need_messages;
	string csv_file;

    po::options_description options("Options");
    options.add_options()
        ("help,h",     			"produce this help message")
        ("brokers,b",  			po::value<string>(&brokers)->required(),
                       	   	   	"the kafka broker list")
        ("input_topics,i",    	po::value< vector<string> >(&input_topic_names)->required(),
                       	   	   	"the topics for reading messages")
		("output_topics,o",    	po::value< vector<string> >(&output_topic_names)->required(),
								"the topics for writing messages")
        ("group-ids,g", 		po::value< vector<string> >(&group_ids)->required(),
                       	   	    "the consumer group ids")
	    ("number_of_workers,n", po::value<int>(&N)->required(),
							  	"count of workers")
		("size_of_buffer,s", 	po::value<int>(&size_of_buffer)->required(),
							  	"size of buffer")
		("size_of_batch,t", 	po::value<int>(&size_of_batch)->required(),
							  	"size of batch")
		("limit_count_before,l", 	po::value<int>(&need_messages)->required(),
							  	"count of message which need for reading batch")
		("path_csv,c", 	        po::value<string>(&csv_file)->required(),
							  	"name with path of csv file")
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

	// Create consumers
	vector<Consumer> consumers;
	for (int i = 0; i < input_topic_names.size(); i++) {
		// Construct the configuration
		Configuration config_cons = {
			{ "metadata.broker.list", brokers },
			{ "group.id", group_ids[i] },
			// Disable auto commit
			{ "enable.auto.commit", false }
		};
		// Create the consumer
		consumers.emplace_back(config_cons);
		// Subscribe to the topic
		consumers[i].subscribe({ input_topic_names[i] });
	}

	// Create producer
	Configuration config_prod = {
			{ "metadata.broker.list", brokers }
		};
	cppkafka::BufferedProducer<string> producer(config_prod);

	// Create ring buffer
	bounded_buffer< std::pair<true_input_type, bool>* > ring_buffer(size_of_buffer);

	// Create mutex and vector for csv_file
	boost::mutex csv_mutex;
	vector<daily_data> act_data;

	// Create manager thread
	boost::thread manager_t{manager, std::ref(running), std::ref(csv_file), std::ref(csv_mutex), std::ref(act_data)};

	// Create threads
	boost::thread_group thrs;
	for(int i = 0; i < N; i++) {
		boost::thread *t = new boost::thread(worker_thread, std::ref(running), std::ref(*buf), std::ref(producer), output_topic_name);
		thrs.add_thread(t);
	}

	// Create keepers of messages
	std::list< std::pair< std::pair<true_input_type, bool>, Message& > list_of_messages;
	std::vector< std::list< std::vector<Message> > > msgs(input_topic_names.size());
	std::vector< std::list<int> > counts_of_handled_msgs(input_topic_names.size());

	// Now read lines and write them into kafka
	while (running) {
		// Try to consume a message
		if (ring_buffer.cur_count() < need_messages) {
			// Get batch
			msgs[0].push_back(consumers[0].poll_batch(size_of_batch));
			counts_of_handled_msgs[0].push_back(0)

			// Loop-Handler getting batch
			int new_size(msgs[0].back().size());
			for (int i = 0; i < new_size; i++) {
				Message & cur_msg = msgs[0].back()[i];
				// Check message
				if (!cur_msg || cur_msg.get_error()) {
					counts_of_handled_msgs[0].back()++;
					continue;
				}
				// De-serialization
				true_input_type new_value = deserialization(cur_msg.get_payload());
				if (!validation(new_value)) {
					counts_of_handled_msgs[0].back()++;
					continue;
				}
				new_value.set_topic(input_topic_names[0]);
				
			}

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
