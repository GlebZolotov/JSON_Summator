// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#pragma once

#include <vector>

#include "mat_array_type.hpp"
#include "mat_device.hpp"

struct matvar_t;

namespace mad_min::io::mat
{

class BaseArray : public NonCopyableNonMovable
{
public:
    virtual
    ~BaseArray();

    ArrayType
    type() const;

    size_t
    rank() const;

    size_t
    size() const;

    size_t
    dim(size_t i) const;

    template <typename T>
    std::vector<T>
    dims() const
    {
        std::vector<T> dims;
        dims.reserve(rank());

        for (size_t i = 0; i < rank(); ++i)
        {
            dims.push_back(dim(i));
        }

        return dims;
    }

    bool
    is_complex() const;

    void*
    re_data() const;

    void*
    im_data() const;

    template <typename S, typename... Args>
    void
    throw_error(const S& format_str, const Args&... args) const
    {
       m_device.throw_error("variable '{}' {}", m_name, format(format_str, args...));
    }

protected:
    BaseArray(BaseDevice& device, std::string_view name);

    void
    check_rank(size_t rank) const;

    void
    check_complex() const;

    void
    transpose(void* from, void*& to, bool reverse);

protected:
    BaseDevice& m_device;
    std::string m_name;
    matvar_t* m_var = nullptr;
    ArrayType m_type = ArrayType::Unsupported;

    // FIXME add m_ prefix and rename
    void* original_re_data = nullptr;
    void* original_im_data = nullptr;
    void* transposed_re_data = nullptr;
    void* transposed_im_data = nullptr;
    bool free_transposed = false;
};

class InputArray : public BaseArray
{
public:
    InputArray(InputDevice& device, std::string_view name);

    ~InputArray() override;

    using BaseArray::check_complex;

    void
    read_data(bool row_major_order = true);
};

class OutputArray : public BaseArray
{
public:
    OutputArray(
        OutputDevice& device,
        std::string_view name,
        ArrayType type,
        std::vector<size_t> dims,
        void* re_data,
        void* im_data = nullptr);

    ~OutputArray() override;

    void
    write_data(bool row_major_order = true);
};

}
