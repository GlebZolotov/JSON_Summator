#include "task_former/task_former.hpp"

std::string data_path = "../data";
int main() {
    std::string input_json = data_path + "/input_params.json";
    std::string close = data_path + "/close.csv";
    std::string returns = data_path + "/returns.csv";
    std::string meta = data_path + "/meta.csv";

    TaskFormer task(input_json, close, returns, meta);
    task.formTask();

    task.runTask();

    nlohmann::json output_results = task.getResults();
    std::ofstream file(data_path + "/ouput_results.json");
    file << std::setw(4) << output_results;
}