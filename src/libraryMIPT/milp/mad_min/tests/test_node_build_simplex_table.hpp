// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#pragma once

#include "tests.hpp"

namespace mad_min::tests::mat
{

class NodeBuildSimplexTable : public MatTest
{
protected:
    virtual
    bool
    test_row(MatFile& file, uint32_t row, bool verbose) override;
};

}

namespace mad_min::tests::hdf
{

class NodeBuildSimplexTable : public HdfTest
{
protected:
    virtual
    bool
    test_row(const H5Easy::File& file, uint32_t row, bool verbose) override;
};

}