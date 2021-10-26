// Copyright (c) 2021 - present
//
// Anton Anikin <anton@anikin.xyz>
// Alexander Gornov <gornov.a.yu@gmail.com>
//
// All rights reserved.

#include "tests.hpp"

namespace mad_min::tests::mat
{

bool
MatTest::run(const std::string& file_name, bool verbose)
{
    std::unique_ptr<MatFile> file;

    print("{}:\n", file_name);
    try
    {
        file = std::make_unique<io::mat::InputDevice>(file_name);
    }
    catch (const std::exception& e)
    {
        print("    {}\n", e.what());
        return false;
    }

    const auto count = io::mat::load<uint32_t>(*file, "count");

    bool ok = true;
    bool row_ok = true;

    for (uint32_t row = 1; row <= count; ++row)
    {
        if (row_ok == false)
        {
            print("\n");
        }

        print("\r  {}/{} ", row, count);
        std::fflush(stdout);

        try
        {
            row_ok = test_row(*file, row, verbose);
        }
        catch (const std::exception& e)
        {
            row_ok = false;
            print("\n    {}", e.what());
        }

        ok &= row_ok;
    }
    print("\n");

    return ok;
}

}

namespace mad_min::tests::hdf
{

bool
HdfTest::run(const std::string& hdf_file_name, bool verbose)
{
    std::unique_ptr<H5Easy::File> file;

    print("{}:\n", hdf_file_name);
    try
    {
        file = std::make_unique<H5Easy::File>(hdf_file_name, H5Easy::File::ReadOnly);
    }
    catch (const std::exception& e)
    {
        print("\n    {}", e.what());
        return false;
    }

    const auto count = H5Easy::load<uint32_t>(*file, "count");

    bool ok = true;
    bool row_ok = true;

    for (uint32_t row = 1; row <= count; ++row)
    {
        if (row_ok == false)
        {
            print("\n");
        }

        print("\r  {}/{} ", row, count);
        std::fflush(stdout);

        try
        {
            row_ok = test_row(*file, row, verbose);
        }
        catch (const std::exception& e)
        {
            row_ok = false;
            print("\n    {}", e.what());
        }

        ok &= row_ok;
    }
    print("\n");

    return ok;
}

}
