option(HDF5_TESTS "Enable HDF5-based tests" OFF)

if (HDF5_TESTS)
    add_definitions(-DHDF5_TESTS)
endif()

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 8.0.1
  GIT_SHALLOW TRUE
  GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(fmt)

set(MADMIN_SOURCES
    advance_tableau.cpp
    best_integer_solution.cpp
    common.cpp
    csv.cpp
    integer_local_improve.cpp
    maximize_return_on_cube.cpp
    node.cpp
    parameters.cpp
    quick_return_to_feasibility.cpp
    return_table_to_feasibility.cpp
    tighten_bounds.cpp
    node_build_simplex_table.cpp
    solver.cpp
)

if (HDF5_TESTS)
    FetchContent_Declare(
        HighFive
        GIT_REPOSITORY https://github.com/BlueBrain/HighFive.git
        GIT_TAG v2.3.1
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    set(HIGHFIVE_UNIT_TESTS OFF)
    set(HIGHFIVE_EXAMPLES OFF)
    set(HIGHFIVE_USE_BOOST OFF)
    set(HIGHFIVE_BUILD_DOCS OFF)
    set(HIGHFIVE_USE_EIGEN OFF) # FIXME explain
    FetchContent_MakeAvailable(HighFive)

    FetchContent_Declare(
        matio
        GIT_REPOSITORY https://github.com/tbeu/matio
        GIT_TAG v1.5.21
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(matio)

    list(APPEND MADMIN_SOURCES
        tests/tests.cpp
        tests/test_advance_tableau.cpp
        tests/test_integer_local_improve.cpp
        tests/test_maximize_return_on_cube.cpp
        tests/test_parameters.cpp
        tests/test_quick_return_to_feasibility.cpp
        tests/test_return_table_to_feasibility.cpp
        tests/test_tighten_bounds.cpp

        tests/test_node_build_simplex_table.cpp
        tests/test_node_constructor.cpp
        tests/test_node_bifurcate.cpp

        io/bits/mat_array.cpp
        io/bits/mat_array_type.cpp
        io/bits/mat_device.cpp
        io/bits/mat_std_types.cpp
        io/bits/mat_eigen.cpp
    )
endif()

add_library(madmin SHARED ${MADMIN_SOURCES})

#target_compile_options(madmin
    #PUBLIC -O2 -Wall -Wextra -Wpedantic -Wdouble-promotion -Wfloat-conversion
    #PUBLIC -DH5_USE_EIGEN
#)

target_link_libraries(madmin PUBLIC
    Eigen3::Eigen
    fmt::fmt-header-only
    #fmt::fmt
)

add_executable(madmin_demo2
    demo2.cpp
    task_former.cpp
)

target_link_libraries(madmin_demo2 PRIVATE
    madmin
    json_csv_lib
)

set_target_properties(madmin_demo2 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

if (HDF5_TESTS)
    target_link_libraries(madmin PUBLIC
        HighFive
        matio
    )

    add_executable(madmin_demo
        demo.cpp
    )

    target_link_libraries(madmin_demo PRIVATE
        madmin
    )

    set_target_properties(madmin_demo PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    )

    #add_executable(madmin_test
        #main.cpp
    #)

    #target_link_libraries(madmin_test PRIVATE
        #madmin
    #)

    #set_target_properties(madmin_test PROPERTIES
        #RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    #)
endif()
