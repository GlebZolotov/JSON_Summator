#include "MADmax_integer_local_improve.hpp"
#include "params.hpp"
#include "utils.hpp"

#include <cmath>

using namespace Eigen;

ReturnValue_localImprove MADmax_integer_local_improve(CommonParams_MADmax* commonParameters, VectorXi32 nu) {
    ReturnValue_localImprove ret;
    VectorXd nu_double = nu.cast<double>();
    double c = nu_double.transpose() * commonParameters->params->Gr;
    double upperwslack = 1 - c;
    double lowerwslack = c - commonParameters->params->w_min;

    if(std::min(upperwslack, lowerwslack) < 0) {
        ret.s = false;
        return ret;
    }

    double old_variance = computeMADvariance(commonParameters, nu_double);
    if(old_variance > commonParameters->gamma_mad) {
        auto Gr_size = commonParameters->params->Gr.size();
        auto nu_size = nu_double.size();

        VectorXd delta_Gr(Gr_size*2);
        delta_Gr << commonParameters->params->Gr, (-1)*commonParameters->params->Gr;

        MatrixXd delta_nu(nu_size, nu_size*2);
        delta_nu << MatrixXd::Identity(nu_size, nu_size), (-1)*MatrixXd::Identity(nu_size, nu_size);
        while(true) {
            VectorXd delta_numax(nu_size*2);
            delta_numax << commonParameters->params->nu_max.cast<double>() - nu_double, nu_double;

            std::vector<int> feas_ind;
            for(auto i = 0; i < nu_size*2; i++) {
                if(delta_numax(i) > 0 && delta_Gr(i) <= upperwslack && -delta_Gr(i) <= lowerwslack)
                    feas_ind.push_back(i);
            }
            VectorXd Mnu = commonParameters->M*nu_double;
            MatrixXd Mnu_update = Mnu*MatrixXd::Ones(1, feas_ind.size()) + commonParameters->M*delta_nu(all, feas_ind);
            RowVectorXd variance_update = Mnu_update.cwiseAbs().colwise().mean();
            int indminvar;
            double minvar = variance_update.minCoeff(&indminvar);
            if(old_variance <= minvar) //no improvement in variance
                break;
            else 
            {
                old_variance = minvar;
                nu_double = nu_double + delta_nu.col(feas_ind[indminvar]);
                upperwslack = upperwslack - delta_Gr(feas_ind[indminvar]);
                lowerwslack = lowerwslack + delta_Gr(feas_ind[indminvar]);
            }
            if(old_variance <= commonParameters->gamma_mad) //variance constraint satusfied
                break;
        }
    }
    if(old_variance > commonParameters->gamma_mad) {
        ret.s = false;
        return ret;
    }

    ret.s = true;
    // start local descent
    ret.value = commonParameters->params->rbar*nu_double;
    //there are n candidate steps, depending on the signs of rbar_i
    Eigen::RowVectorXd sign_rbar = commonParameters->params->rbar.cwiseSign();
    Eigen::RowVectorXd sign_Gr = commonParameters->params->Gr.transpose().cwiseProduct(sign_rbar);
    Eigen::MatrixXd J = sign_rbar.asDiagonal(); //CHECK THIS
    while(true) {
        //find variations not violating feasibility
        std::vector<int> feas_ind;
        for(int i=0; i<sign_rbar.size(); ++i)
        if( ((nu_double.transpose() +sign_rbar)(i) >= 0) &&  
            ((nu_double.transpose() +sign_rbar)(i) <= commonParameters->params->nu_max.transpose().cast<double>()(i)) &&
            (sign_Gr(i) <= upperwslack) && (-sign_Gr(i) <= lowerwslack)) {//OR & <- CHECK
            feas_ind.push_back(i);
        }

        for (auto& count_feas : Utils::matlab_colon<int>(feas_ind.size()-1, 0, -1)) {
            if (computeMADvariance(commonParameters, nu_double + J.col(feas_ind[count_feas])) > commonParameters->gamma_mad) {
                feas_ind.erase(feas_ind.begin() + count_feas); //CHECK!
            }
        }
        if(feas_ind.empty()) {
            break; //if we cannot improve the solution we escape the loop
        }

        //choose the index whose change leads to the largest increase in the objective
        Eigen::RowVectorXd::Index maxind;
        double ma = commonParameters->params->rbar(feas_ind).cwiseAbs().maxCoeff(&maxind);
        ret.value += ma;
        int ind = feas_ind[maxind];
        nu_double(ind)+= sign_rbar(ind);
        upperwslack -= sign_Gr(ind);
        lowerwslack += sign_Gr(ind); 
    }
    ret.nu_local = nu_double.cast<int32_t>();
    return ret;
}