// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#include <matio.h>

#include "mat_device.hpp"

namespace mad_min::io::mat
{

BaseDevice::BaseDevice(std::string_view file_name, bool read_only)
    : m_name(file_name)
{
    if (read_only)
    {
        m_file = Mat_Open(m_name.c_str(), MAT_ACC_RDONLY);
        if (m_file == nullptr)
        {
            throw_error("unable to open for reading");
        }
    }
    else
    {
        m_file = Mat_CreateVer(m_name.c_str(), NULL, MAT_FT_MAT5);
        if (m_file == nullptr)
        {
            throw_error("unable to open for writing");
        }
    }
}

BaseDevice::~BaseDevice()
{
    Mat_Close(m_file);
}

std::string_view
BaseDevice::name() const
{
    return m_name;
}

InputDevice::InputDevice(std::string_view file_name)
    : BaseDevice(file_name, true)
{
}

InputDevice::~InputDevice()
{
}

OutputDevice::OutputDevice(std::string_view file_name)
    : BaseDevice(file_name, false)
{
}

OutputDevice::~OutputDevice()
{
}

}
