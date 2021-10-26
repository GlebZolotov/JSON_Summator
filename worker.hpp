/*
 * worker.hpp
 *
 *  Created on: 4 сент. 2021 г.
 *      Author: gleb
 */

#ifndef WORKER_HPP_
#define WORKER_HPP_

#include <google/protobuf/util/json_util.h>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <thread>
#include "input_data.pb.h"
#include "output_data.pb.h"
#include "logging.hpp"
#include "rapidcsv.h"
#include "mad_solution.hpp"
#include "mad_statement.hpp"

#define RETURNS_IND 0
#define META_IND 1
#define CLOSE_IND 2
#define CSV_COUNT_FILES 3


typedef input_proto::InputData true_input_type;
typedef output_proto::OutputData true_output_type;

true_output_type worker(true_input_type & input_tree);
true_input_type deserialization(const std::string & input_message);
MadStatement & construct_input_for_solver(const true_input_type & in_m, MadStatement & in_s, std::vector<rapidcsv::Document> & csv_data, std::map<std::string, int> & stock_size);
std::string serialization(true_output_type & output_tree);
true_output_type & construct_output_from_solver(true_output_type & out_m, const true_input_type & in_m, const MadSolution & out_s, std::map<std::string, int> & stock_size);
bool validation(true_input_type & input_tree);

template<typename T>
std::map<std::string, std::vector<T>> getDataFromTable(rapidcsv::Document& Table, const std::string& rowName, const std::string& columnName, int rows=1) {
    std::map<std::string, std::vector<T>> data;
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

#endif /* WORKER_HPP_ */
