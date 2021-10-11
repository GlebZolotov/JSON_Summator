/*
 * worker.cpp
 *
 *  Created on: 4 сент. 2021 г.
 *      Author: gleb
 */

#include "worker.hpp"

true_input_type deserialization(const std::string & input_message) {
	true_input_type doc;
	google::protobuf::util::JsonParseOptions options;
    JsonStringToMessage(input_message, &doc, options);
	return doc;
}

std::string serialization(true_output_type & output_data) {
	std::string res;
	google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true;
    options.always_print_primitive_fields = true;
    options.preserve_proto_field_names = true;
    MessageToJsonString(output_data, &res, options);
	return res;
}

bool validation(true_input_type & input_data) {
	if (input_data.rqid() == std::string()) return false;
	if (input_data.model() == std::string()) return false;
	if (input_data.market() == std::string()) return false;
	if (input_data.universe_size() == 0) return false;
	if (!input_data.has_p()) return false;
	return true;
}

true_output_type worker(true_input_type & input_data) {
	true_output_type res;
	res.set_rqid(input_data.rqid());
	res.set_model(input_data.model());
	res.set_market(input_data.market());
	res.set_currency(input_data.currency());

	return res;
}



