/*
 * dailydata.cpp
 *
 *  Created on: 11 окт. 2021 г.
 *      Author: gleb
 */

#include "dailydata.hpp"

daily_data::daily_data(){};

daily_data::daily_data(std::string t, double v, bool i): ticker(t), value(v), is_triggered(i) {};