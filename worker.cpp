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
	std::this_thread::sleep_for(std::chrono::milliseconds(10000));

	return res;
}

MadStatement & construct_input_for_solver(const true_input_type & in_m, MadStatement & in_s) {
	in_s.timeRate = Eigen::MatrixXd::Random(in_m.universe_size(), in_m.p().t()); // ?
	in_s.meanRate.resize(in_m.universe_size());
	in_s.maxWeight.resize(in_m.universe_size());
	in_s.lotSize.resize(in_m.universe_size());
	in_s.close.resize(in_m.universe_size());
	for (int i = 0; i < in_m.universe_size(); i++) {
    	in_s.meanRate[i] = i / 0.45; // ?
    	in_s.maxWeight[i] = in_m.p().p_max(); // ?
		in_s.lotSize[i] = i; // ?
		in_s.close[i] = i / 0.8; // ?
	}
    in_s.minReturn = in_m.p().min_return();
    in_s.maxRisk = in_m.p().max_risk();
    in_s.capital = in_m.p().capital();
    in_s.maxSize = 1.; // ?
    in_s.minSize = 0.; // ?
    in_s.objectiveUserLimit = in_m.user_obj_limit();
    in_s.iterationLimit = in_m.iteration_limit();
    in_s.nodeLimit = in_m.node_limit();
    in_s.timeLimit = in_m.time_limit();

	return in_s;
}

true_output_type & construct_output_from_solver(true_output_type & out_m, const true_input_type & in_m, const MadSolution & out_s) {
	out_m.set_rqid(in_m.rqid());
	out_m.set_model(in_m.model());
	out_m.set_market(in_m.market());
	out_m.set_currency(in_m.currency());
	out_m.set_total_cost(out_s.totalCost);
	out_m.set_obj(out_s.objectiveValue);
	//out_m.set_commission();
	for (int i = 0; i < in_m.universe_size(); i++) {
		(*(out_m.mutable_lots_number()))[in_m.universe()[i]] = out_s.numLots[i];
		(*(out_m.mutable_weights()))[in_m.universe()[i]] = out_s.weights[i];
		// (*(out_m.mutable_prices()))[in_m.universe()[i]] = out_s.prices[i]; ???
		// (*(out_m.mutable_quantity()))[in_m.universe()[i]] = out_s.quantity[i]; ???
	}
	std::vector<output_proto::OutputData_Statuses> statuses {
		output_proto::OutputData_Statuses_NOT_STARTED,
        output_proto::OutputData_Statuses_STARTED,
        output_proto::OutputData_Statuses_OPTIMAL_FOUND,
        output_proto::OutputData_Statuses_INCORRECT_PROBLEM, 
        output_proto::OutputData_Statuses_UNBOUNDED_PROBLEM,
        output_proto::OutputData_Statuses_ITERATION_LIMIT_REACHED,
        output_proto::OutputData_Statuses_NODE_LIMIT_REACHED,
        output_proto::OutputData_Statuses_TIME_LIMIT_REACHED,
        output_proto::OutputData_Statuses_INTERRUPTED,
        output_proto::OutputData_Statuses_NUMERICAL_ERROR,
        output_proto::OutputData_Statuses_SUBOPTIMAL_FOUND,
        output_proto::OutputData_Statuses_USER_OBJ_LIMIT_REACHED,
        output_proto::OutputData_Statuses_NOT_SUPPORTED
	};
	out_m.set_status(statuses[static_cast<int>(out_s.status)]);
	out_m.set_iteration_amount(out_s.iterationAmount);
	out_m.set_accuracy(out_s.accuracy);

	return out_m; 
}



