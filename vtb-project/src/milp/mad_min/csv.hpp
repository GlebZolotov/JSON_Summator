// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#pragma once

#include "common.hpp"

namespace mad_min
{

std::tuple<d_matrix /* retG */, d_cvector /* gran */>
load_csv(const std::string& csv_path, const char delimeter = ',');

}
