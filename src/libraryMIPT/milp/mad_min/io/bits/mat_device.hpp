// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#pragma once

#include "../../common.hpp"

struct _mat_t;

namespace mad_min::io::mat
{

class Error : public std::runtime_error
{
public:
    template <typename S, typename... Args>
    Error(const S& format_str, const Args&... args)
        : std::runtime_error(fmt::format(format_str, args...))
    {
    }
};

class NonCopyable
{
protected:
    NonCopyable() {}
    ~NonCopyable() {}

    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};

class NonMovable
{
protected:
    NonMovable() {}
    ~NonMovable() {}

    NonMovable(const NonMovable&&) = delete;
    NonMovable& operator=(const NonMovable&&) = delete;
};

class NonCopyableNonMovable : public NonCopyable, public NonMovable
{
protected:
    NonCopyableNonMovable() {}
    ~NonCopyableNonMovable() {}
};

class BaseDevice : public NonCopyableNonMovable
{
    friend class BaseArray;
    friend class InputArray;
    friend class OutputArray;

public:
    virtual
    ~BaseDevice();

    std::string_view
    name() const;

protected:
    BaseDevice(std::string_view file_name, bool read_only);

    template <typename S, typename... Args>
    void
    throw_error(const S& format_str, const Args&... args) const
    {
        throw Error("{}: {}", m_name, format(format_str, args...));
    }

protected:
    std::string m_name;
    _mat_t* m_file = nullptr;
};

class InputDevice : public BaseDevice
{
public:
    InputDevice(std::string_view file_name);

    ~InputDevice() override;
};

class OutputDevice : public BaseDevice
{
public:
    OutputDevice(std::string_view file_name);

    ~OutputDevice() override;
};

}
