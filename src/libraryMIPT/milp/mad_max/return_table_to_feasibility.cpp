#include "return_table_to_feasibility.hpp"
#include "advance_tableu.hpp"
#include "utils.hpp"

#include <iostream>
#include <numeric>

using namespace Eigen;
bool return_table_to_feasibility(VectorXd delta_slack, MatrixXd& Tab, RowVectorXi32 Bas) {
    //std::clog << "return to feasibility:\n";
    int n = delta_slack.size();
    MatrixXd Tab_tmp(Tab.rows()+2, n+3);
    Tab_tmp.block(0, 0, 1, n+3) << RowVectorXd::Zero(n), -1.0, 0.0, -1.0;
    Tab_tmp(seqN(1, Tab.rows()), seqN(0, n)) = Tab.leftCols(n);
    Tab_tmp(seqN(1, Tab.rows()), n) = -Tab.leftCols(n)*delta_slack;
    Tab_tmp(seqN(1, Tab.rows()), n+1) = VectorXd::Zero(Tab.rows());
    Tab_tmp(seqN(1, Tab.rows()), n+2) = Tab(all, last);
    Tab_tmp.block(1+Tab.rows(), 0, 1, n+3) << RowVectorXd::Zero(n), 1.0, 1.0, 1.0;
    RowVectorXi32 Bas_tmp(Bas.size() + 1);
    Bas_tmp << Bas, n+1; //?
    bool opt = false;
    int count_advance = 0;
    while (!opt) {
        count_advance++;
        AdvanceTable_Result r = advance_tableu(Tab_tmp, Bas_tmp,true);
        opt = r.opt;
    }
    //std::clog << "Phase 1 from parent table: "<< count_advance << " simplex iterations\n";
    Tab = Tab_tmp;
    Bas = Bas_tmp;

    int qz = -1;
    for(int i = 0; i < Bas.size(); ++i)
        if(Bas(i) == n+1) {
            //std::clog << "finded n+1\n";
            qz = i;
            break;
        }
    if((qz >= 0) && (Tab(0, last) > -1e-10)) {
        //z variable is still basic, but reached zero
        //find a non-basic variable which can be made basic and replace z
        //the corresponding element in the row qz has to be positive
        //std::clog << "z variable still basic\n";
        std::vector<int> non_bas(n);
        std::iota(non_bas.begin(), non_bas.end(), 2);
        for(int i = 0; i < Bas.size(); ++i) {
            auto it = std::find(non_bas.begin(), non_bas.end(), Bas(i));
            if(it != non_bas.end()) {
                non_bas.erase(it);
            }
        }
        int jind;
        Tab(qz+2, non_bas).minCoeff(&jind);
        int j = non_bas[jind];
        Tab(qz + 2, all) = Tab(qz+2, all)/Tab(qz+2, j);
        
        std::vector<int> row_ind(Tab.rows());
        std::iota(row_ind.begin(), row_ind.end(), 0);
        row_ind.erase(row_ind.begin() + qz+2);
        Tab(row_ind, all) -= Tab(row_ind, j)*Tab(qz+2, all);
        Bas(qz) = j;
    }
    if(qz < 0) {
        //auxiliary z variable is non-basic and hence zero, feasible point reached
        int qy;
        for(int i = 0; i < Bas.size(); i++) {
            if(Bas(i) == n) {
                //std::clog << "y must be basic. finded\n";
                qy = i;
                break;
            }
        }
        Utils::remove_row(Tab, 0);
        Utils::remove_row(Tab, qy+2); //check it
        Utils::remove_element(Bas, qy); 
        Utils::remove_column(Tab, n);
        Utils::remove_column(Tab, n+1);
        return true;
    } else {
        Tab.resize(0,0);
        Bas.resize(0);
        return false;
    }

}