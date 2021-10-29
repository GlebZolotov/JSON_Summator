// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#include "../parameters.hpp"

#include "test_parameters.hpp"

#define H5_LOAD(name, hdf_type) \
    name = H5Easy::load<hdf_type>(file, format("r_{}_params_{}", row, #name))

#define H5_LOAD_ALL(prefix) \
    /* input */ \
    prefix H5_LOAD(W, uint16_t); \
    prefix H5_LOAD(T, uint16_t); \
    prefix H5_LOAD(bTcoef, double); \
    prefix H5_LOAD(mu0, double); \
    prefix H5_LOAD(wmin, double); \
    prefix H5_LOAD(R, d_matrix); \
    prefix H5_LOAD(Gr, d_cvector); \
    prefix H5_LOAD(numax, u_cvector); \
    /* output */ \
    prefix H5_LOAD(rbar, d_rvector); \
    prefix H5_LOAD(ind_rho_div_Gr, u_rvector); \
    prefix H5_LOAD(M, d_matrix); \
    prefix H5_LOAD(zeroTableau, d_matrix);

#define MAT_LOAD(name, type) \
    name = mad_min::io::mat::load<type>(file, format("r_{}_{}params_{}", row, prefix, #name))

#define MAT_LOAD_ALL(prefix) \
    /* input */ \
    prefix MAT_LOAD(W, uint16_t); \
    prefix MAT_LOAD(T, uint16_t); \
    prefix MAT_LOAD(bTcoef, double); \
    prefix MAT_LOAD(mu0, double); \
    prefix MAT_LOAD(wmin, double); \
    prefix MAT_LOAD(R, d_matrix); \
    prefix MAT_LOAD(Gr, d_cvector); \
    prefix MAT_LOAD(numax, u_cvector); \
    /* output */ \
    prefix MAT_LOAD(rbar, d_rvector); \
    prefix MAT_LOAD(ind_rho_div_Gr, u_rvector); \
    prefix MAT_LOAD(M, d_matrix); \
    prefix MAT_LOAD(zeroTableau, d_matrix);

#define TEST(params, name) row_ok &= is_equal(params->name, name, #params "." #name)

#define TEST_ALL(params) \
    /* input */ \
    TEST(params, W); \
    TEST(params, T); \
    TEST(params, bTcoef); \
    TEST(params, mu0); \
    TEST(params, wmin); \
    TEST(params, R); \
    TEST(params, Gr); \
    TEST(params, numax); \
    /* output */ \
    TEST(params, rbar); \
    TEST(params, ind_rho_div_Gr); \
    TEST(params, M); \
    TEST(params, zeroTableau);

namespace mad_min
{

void
Parameters::h5_save(const std::string& file_name, std::string_view prefix)
{
    H5Easy::File file(file_name, H5Easy::File::Overwrite);

    H5Easy::dump<uint16_t>(file, format("{}/W", prefix), W);
    H5Easy::dump<uint16_t>(file, format("{}/T", prefix), T);
    H5Easy::dump<double>(file, format("{}/bTcoef", prefix), bTcoef);
    H5Easy::dump<double>(file, format("{}/mu0", prefix), mu0);
    H5Easy::dump<double>(file, format("{}/wmin", prefix), wmin);
    H5Easy::dump<d_matrix>(file, format("{}/R", prefix), R);
    H5Easy::dump<d_cvector>(file, format("{}/Gr", prefix), Gr);
    H5Easy::dump<u_cvector>(file, format("{}/numax", prefix), numax);

    H5Easy::dump<d_rvector>(file, format("{}/rbar", prefix), rbar);
    H5Easy::dump<u_rvector>(file, format("{}/ind_rho_div_Gr", prefix), ind_rho_div_Gr);
    H5Easy::dump<d_matrix>(file, format("{}/M", prefix), M);
    H5Easy::dump<d_matrix>(file, format("{}/zeroTableau", prefix), zeroTableau);
}

Parameters::Ptr
Parameters::h5_load(const std::string& file_name, int row)
{
    H5Easy::File file(file_name, H5Easy::File::ReadOnly);

    auto params = std::make_shared<Parameters>();
    H5_LOAD_ALL(params->);
    params->ind_rho_div_Gr.array() -= 1; // FIX indexing, Matlab is 1-based

    return params;
}

Parameters::Ptr
Parameters::mat_load(std::string_view file_name, int row, std::string_view _prefix)
{
    auto file = io::mat::open(file_name);

    std::string prefix;
    if (not _prefix.empty())
    {
        prefix = format("{}_", _prefix);
    }

    auto params = std::make_shared<Parameters>();
    MAT_LOAD_ALL(params->);
    params->ind_rho_div_Gr.array() -= 1; // FIX indexing, Matlab is 1-based

    return params;
}

bool
Parameters::mat_test(std::string_view file_name, int row, std::string_view _prefix)
{
    using namespace tests;

    auto file = io::mat::open(file_name);
    std::string prefix;
    if (not _prefix.empty())
    {
        prefix = format("{}_", _prefix);
    }

    MAT_LOAD_ALL(auto);
    ind_rho_div_Gr.array() -= 1; // FIX indexing, Matlab is 1-based

    bool row_ok = true;
    TEST_ALL(this);

    return row_ok;
}

}

namespace mad_min::tests::hdf
{

bool
ParametersHdfTest::test_row(const H5Easy::File& file, uint32_t row, bool /*verbose*/)
{
    bool row_ok = true;

    // load data from HDF5 file into local variables
    H5_LOAD_ALL(auto);
    ind_rho_div_Gr.array() -= 1; // FIX indexing, Matlab is 1-based

    // load data from HDF5 file into Parameters instance and test it
    auto p1 = Parameters::h5_load(file.getName(), row);
    TEST_ALL(p1);

    // create Parameters instance from loaded data and test it
    auto p2 = std::make_shared<Parameters>(W, T, bTcoef, mu0, wmin, R, Gr, numax);
    TEST_ALL(p2);

    return row_ok;
}

}

namespace mad_min::tests::mat
{

bool
ParametersTest::test_row(MatFile& file, uint32_t row, bool /*verbose*/)
{
    bool row_ok = true;

    std::string prefix;

    // load data from MAT file into local variables
    MAT_LOAD_ALL(auto);
    ind_rho_div_Gr.array() -= 1; // FIX indexing, Matlab is 1-based

    // load data from MAT file into Parameters instance and test it
    auto p1 = Parameters::mat_load(file.name(), row);
    TEST_ALL(p1);

    // create Parameters instance from loaded data and test it
    auto p2 = std::make_shared<Parameters>(W, T, bTcoef, mu0, wmin, R, Gr, numax);
    TEST_ALL(p2);

    return row_ok;
}

}
