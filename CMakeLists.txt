cmake_minimum_required (VERSION 3.8)
project (JSON_Summator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_FLAGS "-Wall -g")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=corei7-avx -funroll-loops") #-finline-functions
SET(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g -Wall -Wextra -Wpedantic")

set(SOURCE_EXE JSON_Summator.cpp)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

#download Eigen library from git
FetchContent_Declare(
  Eigen
  GIT_REPOSITORY https://gitlab.com/cantonios/eigen
  GIT_TAG 3.4.0
  GIT_SHALLOW TRUE
  GIT_PROGRESS TRUE)
set(EIGEN_BUILD_DOC OFF)
# note: To disable eigen tests,
# you should put this code in a add_subdirectory to avoid to change
# BUILD_TESTING for your own project too since variables are directory
# scoped
set(BUILD_TESTING OFF)
set(EIGEN_BUILD_PKGCONFIG OFF)
FetchContent_MakeAvailable(Eigen)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(CppKafka REQUIRED)
find_package(Boost COMPONENTS program_options REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/libraryMIPT)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/libraryMIPT/milp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/libraryMIPT/LP)
add_subdirectory(src/logging)
add_subdirectory(src/libraryMIPT)
add_subdirectory(src/worker)
add_subdirectory(src/manager)
add_subdirectory(src/worker_thread)
add_subdirectory(src/utils)
add_subdirectory(include)

add_executable(main ${SOURCE_EXE})	# Создает исполняемый файл с именем main
target_link_libraries(main PRIVATE Threads::Threads CppKafka::cppkafka ${Boost_LIBRARIES} boost_thread logging_lib manager_lib worker_thread_lib include_lib utils_lib)
