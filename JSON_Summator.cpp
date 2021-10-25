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

std::atomic<int> stop_main = 0;
std::atomic<bool> stop_threads = false;

void signalHandler(int signum) {
    stop_main.store(1);
}

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

	IP7_Client *l_pClient = NULL;
	IP7_Trace *log_trace = init_logger(&l_pClient);
	reg_current_thread(log_trace, "main");

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
	signal(SIGINT, signalHandler);

	// Create consumers
	vector<Consumer*> consumers;
	for (int i = 0; i < input_topic_names.size(); i++) {
		// Construct the configuration
		Configuration config_cons = {
			{ "metadata.broker.list", brokers },
			{ "group.id", group_ids[i] },
			// Disable auto commit
			{ "enable.auto.commit", false }
		};
		// Create the consumer
		Consumer *new_cons = new Consumer(config_cons);
		consumers.push_back(new_cons);
		// Subscribe to the topic
		consumers[i]->subscribe({ input_topic_names[i] });
	}

	// Create producer
	Configuration config_prod = {
			{ "metadata.broker.list", brokers }
		};
	cppkafka::BufferedProducer<string> producer(config_prod);

	// Create ring buffer
	std::vector< bounded_buffer< std::pair<true_input_type, std::atomic<bool> & >* >* > ring_buffers;
	for (int i = 0; i < input_topic_names.size(); i++) {
		bounded_buffer< std::pair<true_input_type, std::atomic<bool> & >* >* buf = new bounded_buffer< std::pair<true_input_type, std::atomic<bool> & >* >(size_of_buffer);
		ring_buffers.push_back(buf);
	}

	// Create mutex and vector for csv_file
	boost::mutex csv_mutex;
	vector<daily_data> act_data;

	// Create manager thread
	boost::thread manager_t{manager, std::ref(stop_threads), std::ref(csv_file), std::ref(csv_mutex), std::ref(act_data)};

	// Create threads
	boost::thread_group thrs;
	for(int i = 0; i < N; i++) {
		boost::thread *t = new boost::thread(worker_thread,
											 std::ref(stop_threads),
											 std::ref(ring_buffers), 
											 std::ref(producer),  
											 output_topic_names, 
											 std::ref(csv_file), 
											 std::ref(csv_mutex), 
											 std::ref(act_data),
											 std::string("worker") + std::to_string(i+1));
		thrs.add_thread(t);
	}

	// Create keepers of messages
	std::vector< std::list< std::pair< std::pair<true_input_type, std::atomic<bool> & >, Message& > > > list_of_messages(input_topic_names.size());
	std::vector< std::list< std::vector<Message> > > msgs(input_topic_names.size());
	std::vector< std::list< std::vector< std::atomic<bool> > > > atomics_msgs(input_topic_names.size());
	std::vector< std::list< int > > counts_of_unhandled_msgs(input_topic_names.size());

	int index_of_topic(0);

	// Now read lines and write them into kafka
	while (!stop_main.load()) {
		// Try to consume a message
		if (ring_buffers[index_of_topic]->cur_count() < need_messages) {
			// Get batch
			msgs[index_of_topic].push_back(consumers[index_of_topic]->poll_batch(size_of_batch));
			// Check empty batch
			if (msgs[index_of_topic].back().size() == 0) {
				msgs[index_of_topic].pop_back();
				index_of_topic++;
				if (index_of_topic == input_topic_names.size()) index_of_topic = 0;
			} else {
				// Loop-Handler getting batch
				int new_size(msgs[index_of_topic].back().size());
				log_trace->P7_INFO(0, TM("Get batch with %d msgs"), new_size);

				atomics_msgs[index_of_topic].emplace_back(new_size);
				for (int i_a = 0; i_a < new_size; i_a++) atomics_msgs[index_of_topic].back()[i_a].store(false);
				counts_of_unhandled_msgs[index_of_topic].push_back(new_size);

				for (int i = 0; i < new_size; i++) {
					Message & cur_msg = msgs[index_of_topic].back()[i];
					std::atomic<bool> & cur_atomic = atomics_msgs[index_of_topic].back()[i];
					// Check message
					if (!cur_msg || cur_msg.get_error()) {
						//counts_of_unhandled_msgs[index_of_topic].back()--;
						cur_atomic.store(true);
						continue;
					}
					// De-serialization
					true_input_type new_value;
					new_value = deserialization(cur_msg.get_payload());

					if (!validation(new_value)) {
						//counts_of_unhandled_msgs[index_of_topic].back()--;
						cur_atomic.store(true);
						continue;
					}
					new_value.set_topic_to_output(output_topic_names[index_of_topic]);

					// Put message into list of messages
					list_of_messages[index_of_topic].emplace_back(std::pair<true_input_type, std::atomic<bool> & >(new_value, cur_atomic), cur_msg);
					std::pair<true_input_type, std::atomic<bool> & > * in_buf = &(list_of_messages[index_of_topic].back().first);
					// Put message into ring buffer
					ring_buffers[index_of_topic]->push_front(in_buf);
					
					log_trace->P7_INFO(0, TM("New message put into ring buffer"));
				}
			}
		}

		// Check handled and sent messages
		for (int index_of_topic_to_check = 0; index_of_topic_to_check < input_topic_names.size(); index_of_topic_to_check++) {
			Message *prev_handled = nullptr;
			int count_to_remove(0);
			log_trace->P7_INFO(0, TM("Start check handled tasks"));
			for (const std::pair< std::pair<true_input_type, std::atomic<bool> & >, Message& > & check_msg : list_of_messages[index_of_topic_to_check]) {
				if (check_msg.first.second.load() && counts_of_unhandled_msgs[index_of_topic_to_check].size() > 0) {
					while (counts_of_unhandled_msgs[index_of_topic_to_check].front() == 0) {
						log_trace->P7_INFO(0, TM("Delete fully-handled batch"));
						msgs[index_of_topic_to_check].pop_front();
						atomics_msgs[index_of_topic_to_check].pop_front();
						counts_of_unhandled_msgs[index_of_topic_to_check].pop_front();
						log_trace->P7_INFO(0, TM("Success in delete fully-handled batch"));
					}
					counts_of_unhandled_msgs[index_of_topic_to_check].front()--;
					prev_handled = &(check_msg.second);
					count_to_remove++;
				} else {
					if (count_to_remove > 0) {
						consumers[index_of_topic_to_check]->commit(*prev_handled);
						log_trace->P7_INFO(0, TM("Commit message (all messages before this handled)"));
					}
					break;
				}
			}
			if (count_to_remove == list_of_messages[index_of_topic_to_check].size() && count_to_remove != 0) consumers[index_of_topic_to_check]->commit(*prev_handled);

			// Delete elements from list of messages
			if (count_to_remove > 0) {
				std::list< std::pair< std::pair<true_input_type, std::atomic<bool> & >, Message& > >::iterator i1=list_of_messages[index_of_topic_to_check].begin();
				std::list< std::pair< std::pair<true_input_type, std::atomic<bool> & >, Message& > >::iterator i2=list_of_messages[index_of_topic_to_check].begin();

				std::advance(i2, count_to_remove);
				log_trace->P7_INFO(0, TM("Remove %d committing messages"), count_to_remove);
				list_of_messages[index_of_topic_to_check].erase(i1, i2);
			}
		}
	}

	stop_threads.store(true);
	log_trace->P7_INFO(0, TM("Join manager"));
	manager_t.join();
	log_trace->P7_INFO(0, TM("Join workers"));
	thrs.join_all();
	log_trace->P7_INFO(0, TM("Clean memory"));
	for (int i = 0; i < input_topic_names.size(); i++) {
		log_trace->P7_INFO(0, TM("Delete consumer"));
		consumers[i]-> unsubscribe();
		//delete consumers[i];
		log_trace->P7_INFO(0, TM("Delete ring_buffer"));
		delete ring_buffers[i];
	}
	google::protobuf::ShutdownProtobufLibrary();
	del_current_thread(log_trace);
	close_logger(l_pClient);
	return 0;
}
