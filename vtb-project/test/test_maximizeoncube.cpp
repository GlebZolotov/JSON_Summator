#include <iostream>
#include <string>

#include "milp/mad_max/MADmaximize.hpp"
#include "milp/mad_max/MADmax_integer_local_improve.hpp"
#include "params.hpp"

template <class T>
void print(std::string text, T val) {
    std::cout << text <<": " << val << std::endl;
}

int main() {
    CommonParams_MADmax* commonParametersMax = new CommonParams_MADmax();
    VectorXi32 nu_lower = VectorXi32::Zero(commonParametersMax->params->numShares);
    VectorXi32  nu_upper = commonParametersMax->params->nu_max;
    std::cout << "maximize_return_on_cube:\n";
    ReturnValues_maximize r = maximize_return_on_cube(commonParametersMax, nu_lower, nu_upper);
    print("nu_upper", nu_upper);
    
    print("Status", r.s);
    print("Value", r.value);
    print("nu", r.nu);
    print("nu_low", r.nu_low);
    print("nu_upper", r.nu_upp);

    std::cout << "MADmax_integer_local_improve:\n";
    ReturnValue_localImprove rl = MADmax_integer_local_improve(commonParametersMax, r.nu_low);
    print("Status", rl.s);
    print("nu_local", rl.nu_local);
    print("Value", rl.value);

    delete commonParametersMax;
}