// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#include "csv.hpp"
#include "parameters.hpp"

#include "tests/test_advance_tableau.hpp"
#include "tests/test_parameters.hpp"
#include "tests/test_tighten_bounds.hpp"
#include "tests/test_return_table_to_feasibility.hpp"
#include "tests/test_quick_return_to_feasibility.hpp"

namespace vtb
{

// FIXME
extern double tt1;
extern double tt2;
extern double tt3;
extern double tt4;

}

int
main(int /*argc*/, char ** /*argv*/)
{
    using namespace mad_min;

    bool verbose = true;
    const std::string h5_path = "../matlab/MADmin/";

//     QuickReturnToFeasibilityHdfTest().run(h5_path + "quick_return_to_feasibility.h5", verbose);
//     ReturnTableToFeasibilityHdfTest().run(h5_path + "return_table_to_feasibility.h5", verbose);
//     AdvanceTableauHdfTest().run(h5_path + "advance_tableau.h5", verbose);
//     ParametersHdfTest().run(h5_path + "parameters.h5", verbose);
//     TightenBoundsHdfTest().run(h5_path + "tighten_boundsMADmin.h5", verbose);

    print("tt1 = {:.4f} ms.\n", tt1);
    print("tt2 = {:.4f} ms.\n", tt2);
    print("tt3 = {:.4f} ms.\n", tt3);
    print("tt4 = {:.4f} ms.\n", tt4);

//     auto [retG, gran] = load_csv("data/sharedata_reduced.csv");
//
//     const auto W = retG.cols();
//     const auto T = retG.rows();
//     print("W = {} T = {}\n", W, T);
//
//     matrix<uint16_t> numax(W, 1);
//     Parameters params(W, T, 1.0, 0.9, 1.0, retG, gran, numax);

    return 0;
}
