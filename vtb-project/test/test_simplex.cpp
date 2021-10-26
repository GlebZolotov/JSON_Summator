#include <iostream>
#include <Eigen/Dense>

#include "rapidcsv.h"
#include "milp/mad_max/advance_tableu.hpp"

template<typename T>
Eigen::MatrixX<T> getMatrix(rapidcsv::Document doc) {
    Eigen::MatrixX<T> matrix(doc.GetRowCount(), doc.GetColumnCount());
    for(int i = 0; i < matrix.rows(); ++i) {
        std::vector<T> row_vec = doc.GetRow<T>(i);
        Eigen::RowVectorX<T> row = Eigen::Map<Eigen::RowVectorX<T>>(row_vec.data(), row_vec.size());
        matrix.row(i) = row;
    }
    return matrix;
}

int main() {
    rapidcsv::Document bas_before("../test_data/root_test_advanced_before_Bas.csv", rapidcsv::LabelParams(-1, -1));
    RowVectorXi32 Bas = getMatrix<int32_t>(bas_before).row(0);
    rapidcsv::Document tab_before("../test_data/root_test_advanced_before_Tab.csv", rapidcsv::LabelParams(-1, -1));
    Eigen::MatrixXd Tab = getMatrix<double>(tab_before);
    bool opt = false;
    int count_advance = 0;
    while(!opt) {
        count_advance = count_advance +1;
        AdvanceTable_Result ar = advance_tableu(Tab, Bas, true);
        opt = ar.opt;
    }    
    std::clog << "Phase 1 from new table " << count_advance <<" simplex iterations\n";

    rapidcsv::Document bas_after("../test_data/root_test_advanced_after_Bas.csv", rapidcsv::LabelParams(-1, -1));
    rapidcsv::Document tab_after("../test_data/root_test_advanced_after_Tab.csv", rapidcsv::LabelParams(-1, -1));
    RowVectorXi32 Bas_after = getMatrix<int32_t>(bas_after).row(0);
    Eigen::MatrixXd Tab_after = getMatrix<double>(tab_after);
    for(int i = 0; i < Bas_after.size(); i++) {
        if(Bas(i) != Bas_after(i))
            std::cout << Bas(i) << " " << Bas_after(i) << std::endl;
    }

    for(int i = 0; i < Tab_after.size(); i++) {
        if(std::fabs(Tab(i) - Tab_after(i))  <= 1e-9) {
            //std::cout << "equal: " << Tab(i) << " " << Tab_after(i) << std::endl;
        }
        else {
            std::cout.precision(std::numeric_limits<double>::max_digits10);
            std::cout << "not equal: " << Tab(i) << " " << Tab_after(i) << std::endl;
        }
    }
}