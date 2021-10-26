#include "advance_tableu.hpp"
#include "utils.hpp"
#include <iostream>
#include <limits>

void print(std::vector<int> v) {
    for(auto i : v)
        std::cout << i << std::endl;
}

AdvanceTable_Result advance_tableu(Eigen::MatrixXd& T, RowVectorXi32& B, bool ph1) {
    AdvanceTable_Result result;
    int j; //j is the index of the entering basic column
    double mi = T.row(0).leftCols(T.cols()-1).minCoeff(&j);
    if(mi >= -1e-10) { //CHECK THIS ON simplex
        //program is optimal point
        result.opt = true;
        result.ub = false;
        return result;
    }
    result.opt = false;

    int astart = (ph1) ? 2 : 1;
    
    std::vector<int> posind;
    for(int i = astart; i < T.rows(); ++i) {
        if(T(i, j) > 0.0)  {
            posind.push_back(i);
        }
    }

    if(posind.size() == 0) {
        std::cerr << "problem unbounded\n";
        result.ub = true;
        return result;
    }

    result.ub = false;
    int q = -1; //is the pivot row
    double baratio_min = std::numeric_limits<double>::infinity();
    for(auto& i : posind) {    
        auto baratio = T(i, last)/T(i,j);
        if(baratio < baratio_min) {
            baratio_min = baratio;
            q = i;
        }
    }

    //i = B(q - astart + 1) is the leaving basic column
    B(q-astart) = j;
    T(q, all) = T(q,all)/T(q,j);
    for(int i = 0; i < T.rows(); i++) {
        if(i==q) continue;
        T.row(i) = T.row(i) - T(i, j)*T.row(q);
    }
    return result;
}