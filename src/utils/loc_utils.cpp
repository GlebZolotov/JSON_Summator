#include "loc_utils.hpp"

void write_file(std::string name, const std::string & content) {
    std::ofstream fout(name);
    fout << content;
    fout.close();
}