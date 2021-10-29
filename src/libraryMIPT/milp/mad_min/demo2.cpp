#include "task_former.hpp"


int
main(int /* argc */, char** /* argv */)
{
    std::string data_path = "../data";

    std::string input_json = data_path + "/input_params.json";
//     std::string input_json = data_path + "/input_params1.json";
//     std::string input_json = data_path + "/input_params2.json";
//     std::string input_json = data_path + "/input_params3.json";

    std::string close = data_path + "/close.csv";
    std::string returns = data_path + "/returns.csv";
    std::string meta = data_path + "/meta.csv";

    TaskFormer task(input_json, close, returns, meta);
    task.formTask();

    task.runTask();

    nlohmann::json output_results = task.getResults();
    std::ofstream file(data_path + "/ouput_results.json");
    file << output_results;

    return 0;
}
