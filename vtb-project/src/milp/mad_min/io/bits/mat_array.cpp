// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#include <matio.h>

#include "mat_array.hpp"

namespace mad_min::io::mat
{

ArrayType
array_type(matio_classes type)
{
    switch (type)
    {
        case MAT_C_INT8   : return ArrayType::Int8;
        case MAT_C_INT16  : return ArrayType::Int16;
        case MAT_C_INT32  : return ArrayType::Int32;
        case MAT_C_INT64  : return ArrayType::Int64;

        case MAT_C_UINT8  : return ArrayType::UInt8;
        case MAT_C_UINT16 : return ArrayType::UInt16;
        case MAT_C_UINT32 : return ArrayType::UInt32;
        case MAT_C_UINT64 : return ArrayType::UInt64;

        case MAT_C_SINGLE : return ArrayType::Float;
        case MAT_C_DOUBLE : return ArrayType::Double;

        case MAT_C_EMPTY:
        case MAT_C_CELL:
        case MAT_C_STRUCT:
        case MAT_C_OBJECT:
        case MAT_C_CHAR:
        case MAT_C_SPARSE:
        case MAT_C_FUNCTION:
        case MAT_C_OPAQUE:
            break;
    }

    return ArrayType::Unsupported;
}

struct MatlabType
{
    matio_classes class_type;
    matio_types data_type;
};

MatlabType
matlab_types(ArrayType type)
{
    switch (type)
    {
        case ArrayType::Int8   : return { MAT_C_INT8,  MAT_T_INT8  };
        case ArrayType::Int16  : return { MAT_C_INT16, MAT_T_INT16 };
        case ArrayType::Int32  : return { MAT_C_INT32, MAT_T_INT32 };
        case ArrayType::Int64  : return { MAT_C_INT64, MAT_T_INT64 };

        case ArrayType::UInt8  : return { MAT_C_UINT8,  MAT_T_UINT8  };
        case ArrayType::UInt16 : return { MAT_C_UINT16, MAT_T_UINT16 };
        case ArrayType::UInt32 : return { MAT_C_UINT32, MAT_T_UINT32 };
        case ArrayType::UInt64 : return { MAT_C_UINT64, MAT_T_UINT64 };

        case ArrayType::Float  : return { MAT_C_SINGLE, MAT_T_SINGLE };
        case ArrayType::Double : return { MAT_C_DOUBLE, MAT_T_DOUBLE };

        case ArrayType::Unsupported:
            ;
    }

    return { MAT_C_EMPTY, MAT_T_UNKNOWN };
}

BaseArray::BaseArray(BaseDevice& device, std::string_view name)
    : m_device(device)
    , m_name(name)
{
}

BaseArray::~BaseArray()
{
    Mat_VarFree(m_var);

    if (free_transposed)
    {
        std::free(transposed_re_data);
        std::free(transposed_im_data);
    }
}

ArrayType
BaseArray::type() const
{
    return m_type;
}

size_t
BaseArray::rank() const
{
    return m_var->rank;
}

size_t
BaseArray::size() const
{
    size_t s = 1;
    for (size_t i = 0; i < rank(); ++i)
    {
        s *= dim(i);
    }

    return s;
}

size_t
BaseArray::dim(size_t i) const
{
    assert(i < rank());

    return m_var->dims[i];
}

bool
BaseArray::is_complex() const
{
    return (m_var->isComplex != 0);
}

void*
BaseArray::re_data() const
{
    return transposed_re_data;
}

void*
BaseArray::im_data() const
{
    check_complex();

    return transposed_im_data;
}

void
BaseArray::check_rank(size_t rank) const
{
    // Matlab storage is "column major" but we (and Torch) use "row  major" scheme, so we
    // must use transposing at loading/saving matrices data. Since now we have no "common" method
    // for the "high-order" (rank > 2) transposing we can't properly handle such variables :(

    if ((rank < 1) || (rank > 2))
    {
        throw_error("- rank {} is not supported, sorry", rank);
    }
}

void
BaseArray::check_complex() const
{
    if (is_complex() == false)
    {
        throw_error("is not complex");
    }
}

template <typename T>
void
typed_transpose(const void* void_from, void* void_to, size_t rows, size_t cols)
{
    assert(void_from);
    assert(void_to);

    auto from = static_cast<const T*>(void_from);
    auto to = static_cast<T*>(void_to);

    for (size_t row = 0; row < rows; ++row)
    {
        for (size_t col = 0; col < cols; ++col)
        {
            to[col * rows + row] = from[row * cols + col];
        }
    }
}

void
BaseArray::transpose(void* from, void*& to, bool reverse)
{
    assert(from);

    // here we check for '1' since we save arrays as [1 x n] - see fix_dims()
    if ((rank() < 2) || (dim(0) == 1) || (dim(1) == 1))
    {
        to = from;
        return;
    }

    size_t rows = reverse ? dim(1) : dim(0);
    size_t cols = reverse ? dim(0) : dim(1);

    if (to == nullptr)
    {
        to = std::malloc(m_var->data_size * rows * cols);
        free_transposed = true;
    }

    switch (m_type)
    {
        case ArrayType::Int8:
            return typed_transpose<mat_int8_t>(from, to, rows, cols);

        case ArrayType::UInt8:
            return typed_transpose<mat_uint8_t>(from, to, rows, cols);

        case ArrayType::Int16:
            return typed_transpose<mat_int16_t>(from, to, rows, cols);

        case ArrayType::UInt16:
            return typed_transpose<mat_uint16_t>(from, to, rows, cols);

        case ArrayType::Int32:
            return typed_transpose<mat_int32_t>(from, to, rows, cols);

        case ArrayType::UInt32:
            return typed_transpose<mat_uint32_t>(from, to, rows, cols);

        case ArrayType::Int64:
            return typed_transpose<mat_int64_t>(from, to, rows, cols);

        case ArrayType::UInt64:
            return typed_transpose<mat_uint64_t>(from, to, rows, cols);

        case ArrayType::Float:
            return typed_transpose<float>(from, to, rows, cols);

        case ArrayType::Double:
            return typed_transpose<double>(from, to, rows, cols);

        case ArrayType::Unsupported:
            ;
    }

    throw_error("- unsupported type for transpose");
}

InputArray::InputArray(InputDevice& device, std::string_view name)
    : BaseArray(device, name)
{
    m_var = Mat_VarReadInfo(device.m_file, m_name.c_str());
    if (m_var == nullptr)
    {
        throw_error("not found");
    }

    m_type = array_type(m_var->class_type);
    if (m_type == ArrayType::Unsupported)
    {
        throw_error("- type is not supported");
    }

    check_rank(rank());

    // dims "squeezing"
    if (m_var->rank == 2)
    {
        if ((m_var->dims[0] * m_var->dims[1]) == 1) // [1 x 1] -> [1]
        {
            m_var->dims[1] = 0;
            m_var->rank = 1;
        }
        else if (m_var->dims[1] == 1) // [n x 1] -> [n]
        {
            m_var->dims[1] = 0;
            m_var->rank = 1;
        }
        else if (m_var->dims[0] == 1) // [1 x n] -> [n]
        {
            m_var->dims[0] = m_var->dims[1];
            m_var->dims[1] = 0;
            m_var->rank = 1;
        }
    }
}

InputArray::~InputArray()
{
}

void
InputArray::read_data(bool row_major_order)
{
    if (Mat_VarReadDataAll(m_device.m_file, m_var) != 0)
    {
        throw_error("- unable to read data");
    }

    if (is_complex())
    {
        auto complex_data = static_cast<mat_complex_split_t*>(m_var->data);

        original_re_data = complex_data->Re;
        original_im_data = complex_data->Im;

        if (row_major_order)
        {
            transpose(original_im_data, transposed_im_data, true);
        }
    }
    else
    {
        original_re_data = m_var->data;
    }

    if (row_major_order)
    {
        transpose(original_re_data, transposed_re_data, true);
    }
}

OutputArray::OutputArray(
    OutputDevice& device,
    std::string_view name,
    ArrayType type,
    std::vector<size_t> dims,
    void* re_data,
    void* im_data)

    : BaseArray(device, name)
{
    m_type = type;
    original_re_data = re_data;
    original_im_data = im_data;

    assert(re_data);

    auto matlab_type = matlab_types(type);
    if (matlab_type.class_type == MAT_C_EMPTY)
    {
        throw_error("- type is not supported");
    }

    check_rank(dims.size());

    if ((dims.size() == 1) && (dims[0] != 1))
    {
        // we save arrays as [1 x n]
        dims.insert(dims.begin(), 1);
    }

    if (im_data == nullptr)
    {
        m_var = Mat_VarCreate(
            m_name.c_str(),
            matlab_type.class_type,
            matlab_type.data_type,
            dims.size(),
            dims.data(),
            re_data,
            0);
    }
    else
    {
        auto complex_data = mat_complex_split_t{ re_data, im_data };

        m_var = Mat_VarCreate(
            m_name.c_str(),
            matlab_type.class_type,
            matlab_type.data_type,
            dims.size(),
            dims.data(),
            &complex_data,
            MAT_F_COMPLEX);
    }

    if (m_var == nullptr)
    {
        throw_error("- unable to create");
    }

    if (im_data == nullptr)
    {
        transposed_re_data = m_var->data;
    }
    else
    {
        auto complex_data = static_cast<mat_complex_split_t*>(m_var->data);

        transposed_re_data = complex_data->Re;
        transposed_im_data = complex_data->Im;
    }
}

OutputArray::~OutputArray()
{
}

void
OutputArray::write_data(bool row_major_order)
{
    if (row_major_order)
    {
        transpose(original_re_data, transposed_re_data, false);

        if (is_complex())
        {
            transpose(original_im_data, transposed_im_data, false);
        }
    }

    if (Mat_VarWrite(m_device.m_file, m_var, MAT_COMPRESSION_NONE) != 0)
    {
        throw_error("- unable to write data");
    }
}

}
