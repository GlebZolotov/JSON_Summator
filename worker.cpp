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
    options.ignore_unknown_fields = true;
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
	//if (input_data.market() == std::string()) return false;
	if (input_data.universe_size() == 0) return false;
	if (!input_data.has_parameters()) return false;
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

MadStatement & construct_input_for_solver(const true_input_type & in_m, MadStatement & in_s, std::vector<rapidcsv::Document> & csv_data, std::map<std::string, int> & stock_size) {
	//stock_id: size from file meta.csv. Only stocks from json file
    std::map<std::string, double> stock_closes; //stock_id: [close(T1-t-1), ... close(T1)]. Only stocks from json file and closes(between T1-t and T1) from "close.csv"
    std::map<std::string, std::vector<double>> stock_return; //stock_id: return(T1). Only stocks from json file and return price in T1 day from "close.csv"
    
    for(auto& stock: in_m.universe()) {
        stock_size[stock] = csv_data[META_IND].GetRow<int>(stock)[0];
        try {
            stock_return.merge(getDataFromTable<double>(csv_data[RETURNS_IND], in_m.parameters().t_1(), stock, in_m.parameters().t()));
            double close_value = csv_data[CLOSE_IND].GetCell<double>(stock, in_m.parameters().t_1());
            stock_closes[stock] = close_value;
        } catch(const std::exception& e) {
            std::clog << e.what();
            stock_size.erase(stock);
            stock_return.erase(stock);
        } 
    }

    // m_Input_Solver_Lots = stock_size;

    in_s.minReturn = in_m.parameters().portfolio_return_min();
    in_s.maxRisk = in_m.parameters().portfolio_risk_max();
    in_s.capital = in_m.parameters().capital();
    in_s.minSize = in_m.parameters().weight_min();
    in_s.maxWeight = Eigen::VectorXd::Constant(stock_size.size(), in_m.parameters().portfolio_weight_max());
    //aditional params:
    in_s.objectiveUserLimit = in_m.user_obj_limit();
    in_s.iterationLimit = 100000; //in_m.iteration_limit();
    in_s.nodeLimit = 10000; //in_m.node_limit();
    in_s.timeLimit = 18000; //in_m.time_limit();

    //fill lot vector
    in_s.lotSize.resize(stock_size.size());
    int i = 0;
    for (const auto& [stock_name, count] : stock_size) {
        in_s.lotSize(i++) = count;
    }

    //fill return matrix
    in_s.timeRate.resize(stock_return.size(), stock_return.begin()->second.size()); //NxT;
    i = 0;
    for (const auto& [stock_name, returns] : stock_return) {
        for(unsigned int j = 0; j < returns.size(); j++)
            in_s.timeRate(i, j) = returns[j]*in_s.lotSize(i);
        i++;
    }

    //fill close vector
    in_s.close.resize(stock_closes.size());
    i = 0;
    for (const auto& [stock_name, close_value] : stock_closes) {
        in_s.close(i++) = close_value;
    }

    //fill meanRate vector
    in_s.meanRate = in_s.timeRate.rowwise().mean();

    return in_s;
}

true_output_type & construct_output_from_solver(true_output_type & out_m, const true_input_type & in_m, const MadSolution & out_s, std::map<std::string, int> & stock_size) {
	out_m.set_rqid(in_m.rqid());
	out_m.set_model(in_m.model());
	out_m.set_market(in_m.market());
	out_m.set_currency(in_m.currency());

	out_m.set_total_cost(out_s.totalCost * in_m.parameters().capital());
	out_m.set_obj(out_s.objectiveValue);
	out_m.set_commission(0.1); //Change it
	out_m.set_accuracy(out_s.accuracy);

    if (static_cast<int>(out_s.status) == 2) {
        int i = 0;
        for (const auto& [stock_name, count] : stock_size) {
            (*(out_m.mutable_lots_number()))[stock_name] = out_s.numLots(i);
            unsigned long long int quantity = out_s.numLots(i) * count;
            (*(out_m.mutable_quantity()))[stock_name] = quantity;
            (*(out_m.mutable_weights()))[stock_name] = out_s.weights(i);
            i++;
        }
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

	return out_m; 
}



