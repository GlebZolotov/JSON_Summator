/*
 * logging.cpp
 *
 *  Created on: 5 сент. 2021 г.
 *      Author: gleb
 */

#include "logging.hpp"

namespace attrs   = boost::log::attributes;
namespace expr    = boost::log::expressions;
namespace logging = boost::log;

//Defines a global logger initialization routine
BOOST_LOG_GLOBAL_LOGGER_INIT(my_logger, logger_t)
{
    logger_t lg;

    logging::add_common_attributes();

    logging::add_file_log(
            boost::log::keywords::file_name = SYS_LOGFILE,
            boost::log::keywords::format = (
                    expr::stream << expr::format_date_time<     boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
                    << " [" << expr::attr<     boost::log::trivial::severity_level >("Severity") << "]: "
                    << expr::attr<boost::log::attributes::current_thread_id::value_type >("ThreadID") << " "
                    << expr::smessage
            )
    );

    logging::add_console_log(
            std::cout,
            boost::log::keywords::format = (
                    expr::stream << expr::format_date_time<     boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
                    << " [" << expr::attr<     boost::log::trivial::severity_level >("Severity") << "]: "
                    << expr::attr<boost::log::attributes::current_thread_id::value_type >("ThreadID") << " "
                    << expr::smessage
            )
    );

    logging::core::get()->set_filter
    (
        logging::trivial::severity >= logging::trivial::info
    );

    return lg;
}





