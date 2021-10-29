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

std::string serialization_statement(MadStatement & st) {
    std::stringstream data;
    data << " timeRate:" << std::endl;
    data << st.timeRate << std::endl;
    data << " meanRate:" << std::endl;
    data << st.meanRate << std::endl;
    data << " lotSize:" << std::endl;
    data << st.lotSize << std::endl;
    data << " maxWeight:" << std::endl;
    data << st.maxWeight << std::endl;
    data << " close:" << std::endl;
    data << st.close << std::endl;
    data << " maxRisk: " << st.maxRisk << std::endl;
    data << " minReturn: " << st.minReturn << std::endl;
    data << " maxSize: " << st.maxSize << std::endl;
    data << " minSize: " << st.minSize << std::endl;
    data << " capital: " << st.capital << std::endl;
    data << " accuracy: " << st.accuracy << std::endl;
    data << " nodeLimit: " << st.nodeLimit << std::endl;
    data << " timeLimit: " << st.timeLimit << std::endl;
    data << " objectiveUserLimit: " << st.objectiveUserLimit << std::endl;
    data << " iterationLimit: " << st.iterationLimit << std::endl;
    data << " iterationLimit: " << st.iterationLimit << std::endl;
    return data.str();
}

bool validation(true_input_type & input_data) {
	if (input_data.rqid() == std::string()) return false;
	if (input_data.model() == std::string()) return false;
	//if (input_data.market() == std::string()) return false;
	if (input_data.universe_size() == 0) return false;
	if (!input_data.has_parameters()) return false;
	return true;
}

void worker(true_input_type & input_data, std::vector<rapidcsv::Document> & cur_actual_data, IP7_Trace *log_trace, true_output_type & res_output) {
	MadStatement task;
    std::map<std::string, int> stock_size;

    construct_input_for_solver(input_data, task, cur_actual_data, stock_size);
    write_file(std::string("input_statements/") + input_data.rqid() + std::string(".txt"), serialization_statement(task));
    log_trace->P7_INFO(0, TM("Task ready"));
    ProblemType m_Problem;
    const std::string& model = input_data.model();
    if(model == "mad_max")
        m_Problem = ProblemType::MAD_MAX;
    if(model == "mad_min")
        m_Problem = ProblemType::MAD_MIN;
    if(model == "mad_max_lp")
        m_Problem = ProblemType::MAD_MAX_LP;
    if(model == "mad_min_lp")
        m_Problem = ProblemType::MAD_MIN_LP;

    SimplexMethod linearSolver(1e-16);
    LpSolver lpSolver(&linearSolver);
    Solver<MadSolution>* solver;
    log_trace->P7_INFO(0, TM("New solver"));
    switch(m_Problem) {
        case ProblemType::MAD_MAX  :
            solver = new MADMax_Solver(&task);
            break; 
        case ProblemType::MAD_MIN  :
            solver = new MADMin_Solver(&task);
            break; 
        case ProblemType::MAD_MAX_LP  :
            solver = new MadMaxLpSolver(&task, &lpSolver);
            break; 
        case ProblemType::MAD_MIN_LP  :
            solver = new MadMinLpSolver(&task, &lpSolver);
            break; 
        default : 
            std::clog << "Error on problem type\n";
            return;
    }
    
    log_trace->P7_INFO(0, TM("Start solve thread"));
    std::thread solve_t(&Solver<MadSolution>::Solve, solver);
    // Check status of solving
    MadSolution res;
    int it_time_count(0);
    do {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        res = solver->Current();
        it_time_count++;
        log_trace->P7_INFO(0, TM("I'm wait..."));
        log_trace->P7_INFO(0, TM("Status %d"), res.status);
    } while ((res.status == SolutionStatus::STARTED || res.status == SolutionStatus::NOT_STARTED) && it_time_count < task.timeLimit);

    if (it_time_count >= task.timeLimit) solver->Interrupt();

    solve_t.join();
    log_trace->P7_INFO(0, TM("End solve thread"));
    res = solver->Current();
    delete solver;

    if (it_time_count >= task.timeLimit) res.status = SolutionStatus::TIME_LIMIT_REACHED; 
    
    construct_output_from_solver(res_output, input_data, res, stock_size);
    log_trace->P7_INFO(0, TM("End worker"));
}

MadStatement & construct_input_for_solver(const true_input_type & in_m, MadStatement & in_s, std::vector<rapidcsv::Document> & csv_data, std::map<std::string, int> & stock_size) {
	//stock_id: size from file meta.csv. Only stocks from json file
    std::map<std::string, double> stock_closes; //stock_id: [close(T1-t-1), ... close(T1)]. Only stocks from json file and closes(between T1-t and T1) from "close.csv"
    std::map<std::string, std::vector<double>> stock_return; //stock_id: return(T1). Only stocks from json file and return price in T1 day from "close.csv"
    
    for(auto& stock: in_m.universe()) {
        stock_size[stock] = csv_data[MetaInd].GetRow<int>(stock)[0];
        try {
            stock_return.merge(getDataFromTable<double>(csv_data[ReturnInd], in_m.parameters().t_1(), stock, in_m.parameters().t()));
            double close_value = csv_data[CloseInd].GetCell<double>(stock, in_m.parameters().t_1());
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
    in_s.iterationLimit = 100; //in_m.iteration_limit();
    in_s.nodeLimit = 100; //in_m.node_limit();
    in_s.timeLimit = 1800; //in_m.time_limit();

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



