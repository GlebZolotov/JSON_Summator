/*
 * worker.cpp
 *
 *  Created on: 4 сент. 2021 г.
 *      Author: gleb
 */

#include "worker.hpp"
using namespace std;
using namespace boost::property_tree;

vector<uint8_t> worker(vector<uint8_t> & input_message) {
	ptree doc, res;
	long long int res_sum = 0;
	istringstream in_s(string(input_message.begin(), input_message.end()));
	read_json(in_s, doc);

	for (ptree::value_type &data : doc.get_child("data"))
		res_sum += data.second.get_value<int>();

	res.put("rqId", doc.get<std::string>("rqId"));
	res.put("model", "model3");
	res.put("result", res_sum);

	ostringstream out_s;
	write_json(out_s, res);
	string out_str = out_s.str();
	return vector<uint8_t> (out_str.begin(), out_str.end());
}



