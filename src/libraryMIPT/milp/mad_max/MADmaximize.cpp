#include "MADmaximize.hpp"
#include "params.hpp"
#include "utils.hpp"
#include "milp/mad_max/node.hpp"
#include "LP/simplex.hpp"
#include "advance_tableu.hpp"
#include "quick_return_to_feasibility.hpp"
#include "return_table_to_feasibility.hpp"
#include <vector>
#include <cmath>
#include <algorithm>


ReturnValues_maximize maximize_return_on_cube(CommonParams_MADmax* commonParameters, VectorXi32& nu_lower, VectorXi32& nu_upper) {
    ReturnValues_maximize r;

    double constr_val_lower = nu_lower.cast<double>().transpose()*commonParameters->params->Gr;
    double constr_val_upper = nu_upper.cast<double>().transpose()*commonParameters->params->Gr;
    if ((constr_val_lower > 1.0) || (constr_val_upper < commonParameters->params->w_min)) {
        r.s = false;
        r.value = negative_double_inf;
        r.nu_low.resize(0); //[]
        r.nu_upp.resize(0); //[]
        r.nu.resize(0); //[]
        /*r.index = 0; //[]
        r.index1 = 0; //[]
        r.min_max = -3; //other case!*/
        return r;
    }

    r.s = true;
    std::vector<int> pos_c = Utils::find<double>(commonParameters->params->rbar, [](double x){return x>0.0;}); 
    Eigen::VectorXd nu_c = nu_lower.cast<double>();
    for(auto& pos : pos_c) {
        nu_c[pos] = nu_upper[pos]; //nu maximizing the objective unconditionally
    }
    double c_value = commonParameters->params->Gr.transpose()*nu_c;
    if((c_value >= commonParameters->params->w_min) && (c_value <= 1.0)) {//lambda = 0
        r.value = commonParameters->params->rbar*nu_c;
        r.nu_low = nu_c.cast<int32_t>();
        r.nu_upp = r.nu_low;
        r.nu = nu_c;
        return r;
    }

    if (c_value > 1.0) { //unconditional constraint value too high, lambda < 0
        //scan through critical values from below
        //std::clog << "maximize_on_cube: lambda < 0\n";
        r.nu = nu_lower.cast<double>();
        double constr_val = constr_val_lower;
        while(constr_val <= 1.0) {
            r.index = commonParameters->ind_rho_div_Gr(r.index1);
            r.nu(r.index) = nu_upper(r.index);
            constr_val = constr_val + (nu_upper(r.index) - nu_lower(r.index))*commonParameters->params->Gr(r.index);
            r.index1 = r.index1 + 1;
        }
        r.nu(r.index) = nu_upper(r.index) - (constr_val-1.0)/commonParameters->params->Gr(r.index);
        r.min_max = 1;
    } else { //unconditional constraint value too low, lambda > 0
        //scan through critical values from above
        r.index1 = commonParameters->params->numShares - 1; //?!
        r.nu = nu_upper.cast<double>();
        double constr_val = constr_val_upper;
        while(constr_val >= commonParameters->params->w_min) {
            r.index = commonParameters->ind_rho_div_Gr(r.index1);
            r.nu(r.index) = nu_lower(r.index);
            constr_val = constr_val - (nu_upper(r.index) - nu_lower(r.index))*commonParameters->params->Gr(r.index);
            r.index1--;
        }
        r.nu(r.index) = nu_lower(r.index) + (commonParameters->params->w_min - constr_val)/commonParameters->params->Gr(r.index);
        r.min_max = -1;
    }
    r.value = commonParameters->params->rbar * r.nu;

    if(r.min_max != 0) {
        r.nu_low = r.nu.cast<int32_t>(); //???
        r.nu_low(r.index) = floor(r.nu(r.index));
        r.nu_upp = r.nu_low;
        r.nu_upp(r.index) = r.nu_upp(r.index) + 1.0;
    } 
    return r;
}

using namespace solver;

ReturnValues_MADmaximize MADmaximize_return_on_cube(CommonParams_MADmax* commonParameters, VectorXi32& nu_lower, VectorXi32& nu_upper, Node* node) {
    //first we try to solve the problem quickly without the constraint on the L1 norm
    ReturnValues_MADmaximize r;
    ReturnValues_maximize cube_vals = maximize_return_on_cube(commonParameters, nu_lower, nu_upper);
    r.s = cube_vals.s;
    r.value = cube_vals.value;
    r.nu_low = cube_vals.nu_low;
    r.nu_upp = cube_vals.nu_upp;
    r.nu = cube_vals.nu;
    r.index = cube_vals.index;

    if(!cube_vals.s) { //problem infeasible
        //r.single_index=0;
        r.Tab.resize(0, 0);
        r.Bas.resize(0);
        return r;
    }
    if(computeMADvariance(commonParameters, cube_vals.nu) <= commonParameters->gamma_mad) { //a posteriori the variance constraint is satisfied
        r.single_index = true;
        r.Tab.resize(0, 0);
        r.Bas.resize(0);
        return r;
    }
    //NEXT IS SIMPLEX:
    int n;
    if(!node->isRootNode && node->ParentNode->simplexTableau.size() != 0) {
        r.Tab = node->ParentNode->simplexTableau;
        r.Bas = node->ParentNode->basicSet;
        std::vector<int>& parent_non_determined_nu = node->ParentNode->non_determined_nu;
        n = parent_non_determined_nu.size();
        VectorXd delta_upper = nu_upper(parent_non_determined_nu).cast<double>() 
                              - node->ParentNode->UpperNuBound(parent_non_determined_nu).cast<double>();
        VectorXd delta_lower = node->ParentNode->LowerNuBound(parent_non_determined_nu).cast<double>()
                              - nu_lower(parent_non_determined_nu).cast<double>();
        VectorXd delta_rhs(delta_lower.size() + delta_upper.size() + 3 + 2*commonParameters->params->T);
        delta_rhs << delta_lower, delta_upper, VectorXd::Zero(3 + 2*commonParameters->params->T);
        if (delta_rhs.array().sign().sum() == -1) { //only one equality constraint is tightened
            int j;
            for(int i = 0; i < delta_rhs.size(); ++i) {
                if(delta_rhs(i) < 0) {
                    j = i;
                    break;
                }
            }
            int q;
            for(int i = 0; i < r.Bas.size(); ++i) {
                if(r.Bas(i) == j) {
                    q = i;
                    break;
                }
            }

            bool is_feasible = quick_return_to_feasibility(delta_rhs(j), j, q, r.Tab, r.Bas);
            if(!is_feasible) {
                return r;
            }
        } else { //several equality constraints are tightened
            bool is_feasible = return_table_to_feasibility(delta_rhs, r.Tab, r.Bas);
            if(!is_feasible) {
                return r;
            }
        }
        //we now can eliminate further rows and columns corresponding to equal
        //upper and lower bounds 
        std::vector<int> ind_equal;
        for(int i = nu_upper(parent_non_determined_nu).size()-1; i!=0; --i) {
            if(nu_upper(parent_non_determined_nu)(i) == nu_lower(parent_non_determined_nu)(i)) {
                ind_equal.push_back(i);
            }
        }
        if(ind_equal.size() != 0) {
            //std::clog << "ind_equal not empty\n";
            for(int& k :ind_equal) {
                for(int i = 0; i < r.Bas.size(); i++) {
                    if(r.Bas(i) == n+k) {
                        Utils::remove_row(r.Tab, i+1);
                        Utils::remove_element(r.Bas, i);
                        break;
                    }
                }
                Utils::remove_column(r.Tab, n+k);
                for(int i = n+k+1; i < r.Bas.size(); i++){
                    r.Bas(i)= r.Bas(i) - 1;
                }
                for(int i = 0; i < r.Bas.size(); i++){
                    if(r.Bas(i) == k) {
                        Utils::remove_row(r.Tab, i+1);
                        Utils::remove_element(r.Bas, i);
                        break;
                    }
                }
                Utils::remove_column(r.Tab, k);
                for(int i = k+1; i < r.Bas.size(); i++){
                    r.Bas(i)= r.Bas(i) - 1;
                }
                n = n-1;
            }
        }

    } else {
        n = commonParameters->params->numShares;
        r.Tab = commonParameters->zeroTableau;
        r.Tab(seqN(2, n), last) = nu_lower.cast<double>();
        r.Tab(seqN(2+n, n), last) = nu_upper.cast<double>();

        const double EPSILON = 5.00e-8;
        for(int i = 0; i < r.Tab.rows(); ++i)
            for(int j = 0; j < r.Tab.cols(); ++j) {
                if (abs(r.Tab(i,j)) <  EPSILON) {
                    r.Tab(i,j) = 0;
                }
            }
        
        n = node->non_determined_nu.size();
        // deleting rows and columns of zero slacks
        std::vector<int> ind_det(commonParameters->params->numShares);
        std::iota(ind_det.begin(), ind_det.end(), 0);
        int offset = 0;
        for(int& i : node->non_determined_nu) {
            ind_det.erase(ind_det.begin() + i - offset); //delete indices of determined elements nu_i
            offset++;
        }
        //std::cout << ind_det.size() << std::endl;

        if(ind_det.size() != 0 ) {
            //std::clog << "ind_det not null \n";
            //std::cout << r.Tab.rows() << " " << r.Tab.cols() << std::endl;
            Utils::remove_rows(r.Tab, ind_det, 2); //delete equations on lower bound slack
            Utils::remove_rows(r.Tab, ind_det, 2+n); //delete equations on upper bound slack
            r.Tab(all, last) = r.Tab(all, last) -  r.Tab(all, ind_det)*(nu_lower(ind_det).cast<double>()); //eliminate determined nu_i elements: update right-hand side
            Utils::remove_columns(r.Tab, ind_det); //delete columns corresponding to nu_i and their lower slacks
            Utils::remove_columns(r.Tab, ind_det, n); 
            Utils::remove_columns(r.Tab, ind_det, 2*n); 
            //std::cout << r.Tab.rows() << " " << r.Tab.cols() << std::endl;
        }

        //assembling basic index set
        std::vector<int> pos_a;
        std::vector<int> ind_b;
        for(int i=0; i<commonParameters->M.rows(); ++i)  {
            const auto d = commonParameters->M.row(i).dot(cube_vals.nu);
            if(d >= 0.0) {
                pos_a.push_back(i);
            } else {
                ind_b.push_back(i);
            }
        }

        RowVectorXi32 pos_a_row = Map<RowVectorXi32>(pos_a.data(), pos_a.size());
        RowVectorXi32 ind_b_row = Map<RowVectorXi32>(ind_b.data(), ind_b.size());
        RowVectorXi32 ind_sab(1+pos_a.size()+ind_b.size());
        ind_sab << 3*n+2, 3*(n+1) + pos_a_row.array(), 3*(n+1) + commonParameters->params->T + ind_b_row.array();

        if(cube_vals.min_max == 0) {
            //std::cout << "min_max = 0\n";
            RowVectorXd rbar_non_nu = commonParameters->params->rbar(node->non_determined_nu);
            std::vector<int> rbar_pos = Utils::find(rbar_non_nu, [](double x){return x>0;} );
            VectorXi ind = Utils::matlab_colon(0, n-1).transpose();
            Utils::remove_elements_in_vector(ind, rbar_pos);
            RowVectorXi rbar_nonpos = ind.transpose();
            r.Bas.resize(n + rbar_pos.size() + rbar_nonpos.size() + 2 + ind_sab.size());
            r.Bas << Utils::matlab_colon(0, n-1), n + Map<RowVectorXi>(rbar_pos.data(), rbar_pos.size()).array(), 
                    2*n + rbar_nonpos.array(), 3*n + 3*n + Utils::matlab_colon(0,1).array(), ind_sab;
        } else {
            double kk = (3-cube_vals.min_max)/2;
            RowVectorXi low_row_vec = commonParameters->ind_rho_div_Gr(Utils::matlab_colon<int>(0, cube_vals.index1-1));
            RowVectorXi upp_row_vec = commonParameters->ind_rho_div_Gr(Utils::matlab_colon<int>(cube_vals.index1-1, commonParameters->params->numShares-1));

            std::vector<int> non_det_nu = node->non_determined_nu;
            std::vector<int> low_vec = Utils::get_std_vector<int>(low_row_vec);
            std::vector<int> upp_vec = Utils::get_std_vector<int>(upp_row_vec);
            std::vector<int> low_ind;
            std::vector<int> upp_ind;
            Utils::intersection(non_det_nu, low_vec, &low_ind);
            Utils::intersection(non_det_nu, upp_vec, &upp_ind);
            RowVectorXi32 low_ind_row = Map<RowVectorXi32>(low_ind.data(), low_ind.size());
            RowVectorXi32 upp_ind_row = Map<RowVectorXi32>(upp_ind.data(), upp_ind.size());
            
            r.Bas.resize(n + low_ind_row.size() + upp_ind_row.size() + 1 + ind_sab.size());
            r.Bas << Utils::matlab_colon(0, n-1), n + low_ind_row.array(), 2*n + upp_ind_row.array(), 3*n + kk-1, ind_sab;
            //std::cout << r.Bas <<std::endl;
        }
        //creating simplex table in normal form 
        MatrixXd tmp (2+r.Bas.cols(), r.Tab.rows());
        tmp.block(0, 0, 2, 2).setIdentity();
        tmp.block(2, 0, 2*n+3+commonParameters->params->T, 2).setZero();
        tmp.rightCols(r.Bas.cols()) = r.Tab(all, r.Bas);

        for(int i = 0; i < tmp.rows(); ++i)
            for(int j = 0; j < tmp.cols(); ++j) {
                if (abs(tmp(i,j)) <  EPSILON) {
                    tmp(i,j) = 0;
                }
            }
        
        MatrixXd B = r.Tab;

        //std::cout << tmp << std::endl;
        r.Tab = tmp.inverse() * B; //tmp.llt().solve(B); //MATLAB equivalent mldivide(tmp, Tab) == tmp\Tab
        //removing rows and columns corresponding to nu
        for(int i=0; i < n; i++) {
            Utils::remove_column(r.Tab, 0);
            Utils::remove_element<int32_t>(r.Bas, 0);
        }
        r.Bas.array() -= n;
        for(int i=0; i < n; i++) {
            Utils::remove_row(r.Tab, 2);
        }
        //std::cout << r.Tab << std::endl;
        //minimizing auxiliary function
        bool opt = false;
        int count_advance = 0;
        while(!opt) {
            count_advance = count_advance +1;
            AdvanceTable_Result ar = advance_tableu(r.Tab, r.Bas, true);
            if(ar.ub == true) {
                r.s = false;
                r.Tab.resize(0,0);
                r.Bas.resize(0);
                return r;
            }
            opt = ar.opt;
        }
        //std::clog << "Phase 1 from new table " << count_advance <<" simplex iterations\n";
        for(int i = 0; i < r.Bas.size(); i++) {
            if(r.Bas(i) == 2*n+2) { //?
                //std::clog << "feasible set of the real LP is empty slack of variance constraint is basic\n";
                r.s = false;
                r.Tab.resize(0,0);
                r.Bas.resize(0);
                return r;
            }
        }
        //removing first row corresponding to auxiliary cost
        Utils::remove_row(r.Tab, 0);
        //column corresponding to the gammaMAD slack is inverted back
        //std::cout << "2*n+2:\n" << r.Tab.col(2*n+2) << std::endl;
        //std::cout << "2*n+3:\n" << r.Tab.col(2*n+3) << std::endl;
        r.Tab.col(2*n+2) = -r.Tab.col(2*n+2);
    }
    // maximizing return with constraint on variance
    bool opt = false;
    int count_advance = 0;

    //auto start = std::chrono::high_resolution_clock::now();

    while(!opt) {
        count_advance = count_advance + 1;
        AdvanceTable_Result ar = advance_tableu(r.Tab, r.Bas, false);
            if(ar.ub == true) {
                r.s = false;
                r.Tab.resize(0,0);
                r.Bas.resize(0);
                return r;
            }
        opt = ar.opt;
    }
    //auto finish = std::chrono::high_resolution_clock::now();
    //std::chrono::duration<double> elapsed = finish - start;
    //std::cout << "Simplex:  "<< elapsed.count() <<" sec." << std::endl;
    //std::clog << "Solution phase: " << count_advance <<" simplex iterations\n";
    r.value = r.Tab(0, last);
    /*for(int i = 1; i < r.Tab.rows(); ++i) {
        double ma = std::max(0.0, r.Tab.rightCols(1)(i));
        r.Tab.rightCols(1)(i) = ma; //correct small negative values of the basic variables
    }*/
    r.nu = nu_lower.cast<double>();
    RowVectorXi ind_frac = RowVectorXi::Zero(n);
    int count_frac = 0;
    for(int i = 0; i < n; ++i) {
        int q = -1;
        for(int j = 0; j < r.Bas.size(); j++) {
            if(r.Bas[j] == i) {
                q = j;
                break;
            }
        }
        if(q >= 0) { //nu_i is not at lower bound
            int q1 = -1;
            for(int j = 0; j < r.Bas.size(); j++) {
                if (r.Bas[j] == n+i) {
                    q1=j;
                    break;
                }
            }
            if(q1 < 0) { //nu_i is at upper bound
                r.nu(node->non_determined_nu[i]) = nu_upper(node->non_determined_nu[i]);
            } else { // nu_i is neither at lower nor at upper bound
                r.nu(node->non_determined_nu[i]) += r.Tab.rightCols(1)(q+1); //q+1 ???
                if(std::abs(r.Tab.rightCols(1)(q+1) - std::round(r.Tab.rightCols(1)(q+1))  < 1e-9)) { //by chance the intermediate value is also integer
                    r.nu(node->non_determined_nu[i]) = std::round(r.nu(node->non_determined_nu[i]));
                } else {
                    count_frac++;
                    ind_frac(count_frac) = i;
                }
            }
        }
    }
    if(count_frac == 0) {
        r.index = 0;
        r.single_index = true;
        r.nu_low = r.nu.cast<int32_t>();
        r.nu_upp = r.nu_low;
    } else if(count_frac == 1) {
        r.index = ind_frac(0);
        r.single_index = true;
        VectorXd nu_low_d = r.nu;
        for(int i = 0; i < count_frac; ++i) {
            nu_low_d(ind_frac(i)) = std::floor(r.nu(ind_frac(i)));
            r.nu_upp(ind_frac(i)) = std::ceil(r.nu(ind_frac(i)));
        }
        r.nu_low = nu_low_d.cast<int32_t>();
    } else {
        r.single_index = false;
        VectorXd nu_low_d = r.nu;
        for(int i = 0; i < count_frac; ++i) {
            nu_low_d(ind_frac(i)) = std::floor(r.nu(ind_frac(i)));
            r.nu_upp(ind_frac(i)) = std::ceil(r.nu(ind_frac(i)));
        }
        r.nu_low = nu_low_d.cast<int32_t>();
    }
    return r;
}