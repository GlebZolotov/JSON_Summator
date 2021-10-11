/*
 * dailydata.hpp
 *
 *  Created on: 4 сент. 2021 г.
 *      Author: gleb
 */

#ifndef DAILYDATA_HPP_
#define DAILYDATA_HPP_

#include <string>

class daily_data {
    public:
        std::string ticker;
        double value;
        bool is_triggered;
        daily_data();
        daily_data(std::string t, double v, bool i);

};

#endif /* DAILYDATA_HPP_ */
