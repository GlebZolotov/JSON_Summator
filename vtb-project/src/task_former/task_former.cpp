#include "task_former.hpp"
#include "../milp/MADmax_solver.hpp"
#include "../milp/mad_min/solver.hpp"
#include "../LP/mad_max_lp_solver.hpp"
#include "../LP/mad_min_lp_solver.hpp"

#include <algorithm>
#include <stdexcept>

using nlohmann::json;

void InputTask::setFromJSON(const json &j)
{
    rqId = j.at("rqId");
    model = j.at("model");
    market = j.at("market");
    currency = j.at("currency");
    for(auto &elem: j.at("universe"))
        universe.push_back(elem);
    T_0 = j.at("T_0");
    T_1 = j.at("T_1");

    parameters.max_risk = j.at("parameters").at("max_risk");
    parameters.p_max = j.at("parameters").at("p_max");
    parameters.min_return = j.at("parameters").at("min_return");
    parameters.t = j.at("parameters").at("t");
    parameters.capital = j.at("parameters").at("capital");
    parameters.w_min = j.at("parameters").at("wmin");

    time_limit = j.at("time_limit");
    iteration_limit = j.at("iteration_limit");
    node_limit = j.at("node_limit");
    user_obj_limit = j.at("user_obj_limit");
}

TaskFormer::TaskFormer(std::string pathToJSON, std::string pathToCloseCSV,
               std::string pathToReturnsCSV, std::string pathToMetaCSV) 
{
    std::ifstream file_params(pathToJSON);
    json j_params;
    file_params >> j_params;
    m_Task.setFromJSON(j_params);

    m_ReturnCSV = rapidcsv::Document(pathToReturnsCSV, rapidcsv::LabelParams(0, 0));
    m_MetaCSV = rapidcsv::Document(pathToMetaCSV, rapidcsv::LabelParams(0, 0));
    m_CloseCSV = rapidcsv::Document(pathToCloseCSV, rapidcsv::LabelParams(0, 0));
}

void TaskFormer::formTask() {
    std::string& model = m_Task.model;
    if(model == "mad_max")
        m_Problem = ProblemType::MAD_MAX;
    if(model == "mad_min")
        m_Problem = ProblemType::MAD_MIN;
    if(model == "mad_max_lp")
        m_Problem = ProblemType::MAD_MAX_LP;
    if(model == "mad_min_lp")
        m_Problem = ProblemType::MAD_MIN_LP;

    setSolverData();
}



template<typename T>
using map_data = std::map<std::string, std::vector<T>>;

template<typename T>
map_data<T> getDataFromTable(rapidcsv::Document& Table, std::string& rowName, std::string& columnName, int rows=1) {
    map_data<T> data;
    int rowIdx = Table.GetRowIdx(rowName);
    int columnIdx = Table.GetColumnIdx(columnName);
    if(columnIdx < 0 || rowIdx < 0) {
        throw std::out_of_range("Couldnt find element (" + rowName + ", " + columnName+ ") on table\n");
    }

    while(rows) {
        data[columnName].push_back(Table.GetCell<T>(columnIdx,rowIdx));
        rowIdx--;
        rows--;
    }
    std::reverse(data[columnName].begin(), data[columnName].end());
    return data;
}


MadStatement createMADStatement(meta_t& stock_size, close_t& stock_closes, return_t& stock_return, InputTask& task) 
{
    MadStatement s;
    s.minReturn = task.parameters.min_return;
    s.maxRisk = task.parameters.max_risk;
    s.capital = task.parameters.capital;
    s.minSize = task.parameters.w_min;
    s.maxWeight = Eigen::VectorXd::Constant(task.universe.size(), task.parameters.p_max);
    //aditional params:
    s.objectiveUserLimit = task.user_obj_limit;
    s.iterationLimit = task.iteration_limit;
    s.nodeLimit = task.node_limit;
    s.timeLimit = task.time_limit;

    //fill lot vector
    s.lotSize.resize(stock_size.size());
    int i = 0;
    for (const auto& [stock_name, count] : stock_size) {
        s.lotSize(i++) = count;
    }

    //fill return matrix
    s.timeRate.resize(stock_return.size(), stock_return.begin()->second.size()); //NxT;
    i = 0;
    for (const auto& [stock_name, returns] : stock_return) {
        for(int j = 0; j < returns.size(); j++)
            s.timeRate(i, j) = returns[j]*s.lotSize(i);
        i++;
    }

    //fill close vector
    s.close.resize(stock_closes.size());
    i = 0;
    for (const auto& [stock_name, close_value] : stock_closes) {
        s.close(i++) = close_value;
    }

    //fill meanRate vector
    s.meanRate = s.timeRate.rowwise().mean();

    return s;
}

void TaskFormer::setSolverData() {
    meta_t stock_size; //stock_id: size from file meta.csv. Only stocks from json file
    close_t stock_closes; //stock_id: [close(T1-t-1), ... close(T1)]. Only stocks from json file and closes(between T1-t and T1) from "close.csv"
    return_t stock_return; //stock_id: return(T1). Only stocks from json file and return price in T1 day from "close.csv"
    
    for(auto& stock: m_Task.universe) {
        stock_size[stock] = m_MetaCSV.GetRow<int>(stock)[0];
        try {
            stock_return.merge(getDataFromTable<double>(m_ReturnCSV, m_Task.T_1, stock, m_Task.parameters.t));

            double close_value = m_CloseCSV.GetCell<double>(stock, m_Task.T_1);
            stock_closes[stock] = close_value;
        } catch(const std::out_of_range& e) {
            std::clog << e.what();
        } 
        catch(const std::invalid_argument&) {
            std::clog << "Cant find cell (" << m_Task.T_1 <<", " << stock << ") on 'close' table\n";
            //remove stock in stock_size and stock_returns
            stock_size.erase(stock);
            stock_return.erase(stock);
        }
    }

    m_Input_Solver_Lots = stock_size;
    
    this->m_Data = createMADStatement(stock_size, stock_closes, stock_return, m_Task);
}


void TaskFormer::runTask() 
{
    SimplexMethod linearSolver(1e-16);
    LpSolver lpSolver(&linearSolver);
    Solver<MadSolution>* solver;

    switch(m_Problem) {
    case ProblemType::MAD_MAX  :
      solver = new MADMax_Solver(&m_Data);
      break; 
    case ProblemType::MAD_MIN  :
      solver = new MADMin_Solver(&m_Data);
      break; 
    case ProblemType::MAD_MAX_LP  :
      solver = new MadMaxLpSolver(&m_Data, &lpSolver);
      break; 
    case ProblemType::MAD_MIN_LP  :
      solver = new MadMinLpSolver(&m_Data, &lpSolver);
      break; 
   default : 
      std::clog << "Error on problem type\n";
      return;
    }

    solver->Solve();
    m_Solution = solver->Current();

    delete solver;
}

nlohmann::json TaskFormer::getResults() const
{
    json solution;
    solution["rqId"] = m_Task.rqId;
    solution["model"] = m_Task.model;
    solution["market"] = m_Task.market;
    solution["currency"] = m_Task.currency;

    solution["total_cost"] = m_Solution.totalCost * m_Data.capital;
    solution["obj"] = m_Solution.objectiveValue;
    solution["commission"]= 0.1; //Change it
    solution["accurancy"] = m_Solution.accuracy;

    int i = 0;
    for (const auto& [stock_name, count] : m_Input_Solver_Lots)  {
        solution["lots_number"][stock_name] = m_Solution.numLots(i);
        unsigned long long int quantity = m_Solution.numLots(i) * count;
        solution["quantity"][stock_name] = quantity;
        solution["weights"][stock_name] = m_Solution.weights(i);
        i++;
    }
    solution["status"] = m_Solution.status;

    return solution;
}