/*
 * worker.cpp
 *
 *  Created on: 4 сент. 2021 г.
 *      Author: gleb
 */

#include "worker.hpp"
using namespace std;
using namespace boost::property_tree;

true_input_type deserialization(string & input_message) {
	true_input_type doc;
	istringstream in_s(input_message);
	read_json(in_s, doc);
	return doc;
}

string serialization(true_output_type & output_tree) {
	ostringstream out_s;
	write_json(out_s, output_tree);
	return out_s.str();
}

bool validation(true_input_type & input_tree) {
	ptree::const_assoc_iterator models_pts = input_tree.find("models");
	ptree::const_assoc_iterator data_pts = input_tree.find("data");
	boost::optional<string> rqid_pts = input_tree.get_optional<string>("rqId");
	if(input_tree.not_found() == models_pts || input_tree.not_found() == data_pts || !rqid_pts) {
		return false;
	}
	for (ptree::value_type &data : input_tree.get_child("data")) {
		boost::optional<int> ch_val = input_tree.get_optional<int>("data." + data.first);
		if(!ch_val) {
			return false;
		}
	}
	return true;
}

true_output_type worker(true_input_type & input_tree) {
	true_output_type res;
	long long int res_sum = 0;

	for (ptree::value_type &data : input_tree.get_child("data"))
		res_sum += data.second.get_value<int>();

	res.put("rqId", input_tree.get<string>("rqId"));
	res.put("model", "model3");
	res.put("result", res_sum);

	return res;
}



