#pragma once
#include <string>
#include <vector>

#include "../mad_statement.hpp"
#include "../mad_solution.hpp"
#include "json.hpp"
#include "rapidcsv.h"

using uint = unsigned int;
using universe_t = std::vector<std::string>;
class MadStatement;

struct SolverParams {
    double max_risk; //gamma_mad
    double min_return;
    double p_max;
    uint t;
    double capital;
    double w_min; //TODO what?
};

struct InputTask {
    std::string rqId;
    std::string model;
    std::string market;
    std::string currency;
    universe_t  universe;
    std::string T_0;
    std::string T_1;
    SolverParams parameters;
    double time_limit;
    int iteration_limit;
    int node_limit;
    int user_obj_limit;

    inline void setFromJSON(const nlohmann::json &j);
    inline universe_t getUniverse() const {return universe;}
};

class SolverInputDataMatrix;

class TaskFormer {
public:
    TaskFormer(std::string pathToJSON, std::string pathToCloseCSV,
               std::string pathToReturnsCSV, std::string pathToMetaCSV);

    void formTask();

    void runTask();

    nlohmann::json getResults() const;

private:
    void setSolverData();

    InputTask m_Task;
    rapidcsv::Document m_ReturnCSV;
    rapidcsv::Document m_CloseCSV;
    rapidcsv::Document m_MetaCSV;

    MadStatement m_Data;
    MadSolution m_Solution;
};
