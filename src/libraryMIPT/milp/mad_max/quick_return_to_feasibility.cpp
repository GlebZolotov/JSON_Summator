#include "quick_return_to_feasibility.hpp"
#include "advance_tableu.hpp"
#include "utils.hpp"

#include <iostream>

//check q and j indexes!
using namespace Eigen;
bool quick_return_to_feasibility(double delta, int j, int q, MatrixXd& Tab, RowVectorXi32& Bas) {
    //std::clog << "quick_return_to_feasibility:\n";
    Tab.rightCols(1)(q+1) += delta;
    if(Tab.rightCols(1)(q+1) < 0) {
        Tab.row(q+1) = -Tab.row(q+1);
        Tab.col(j) = -Tab(all, j);
        //adding row corresponding to auxiliary cost function
        MatrixXd Tab_tmp(Tab.rows()+1, Tab.cols());
        Tab_tmp << -Tab.row(q+1), Tab;
        Tab_tmp(0, j) = 0.0;
        //solving auxiliary program
        bool opt = false;
        int count_advance = 0;
        while (!opt) {
            count_advance++;
            AdvanceTable_Result r = advance_tableu(Tab_tmp,Bas,true);
            opt = r.opt;
        }
        //std::clog << "Phase 1 from parent table: "<< count_advance << " simplex iterations\n";
        Tab = Tab_tmp;
        if(Tab(0, last) < -1e-10) {
            Tab.resize(0,0);
            Bas.resize(0);
            return false;
        } else {
            Utils::remove_row(Tab, 0);
            Tab(all, j) = -Tab(all, j);
            for(int i = 0; i < Bas.size(); i++) {
                if(Bas(i) == j) {//variable j is still basic
                    Tab(q+1, all) = -Tab(q+1, all);
                    Tab.rightCols(1)(q+1) = 0;
                    break;
                }
            }
        }
    }
    return true;
}