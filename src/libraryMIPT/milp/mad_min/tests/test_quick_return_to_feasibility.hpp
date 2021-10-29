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

class QuickReturnToFeasibilityTest : public MatTest
{
protected:
    virtual
    bool
    test_row(MatFile& file, uint32_t row, bool verbose) override;
};

}
