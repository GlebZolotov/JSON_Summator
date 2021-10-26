cmake_minimum_required (VERSION 3.10)
project (optimizer VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_FLAGS "-Wall -g")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=corei7-avx -funroll-loops") #-finline-functions
SET(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g -Wall -Wextra -Wpedantic")

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
 
#find_package (Eigen3 3.4 REQUIRED NO_MODULE)
#file(COPY ${CMAKE_SOURCE_DIR}/data DESTINATION ${CMAKE_BINARY_DIR})
#file(COPY ${CMAKE_SOURCE_DIR}/test/test_data DESTINATION ${CMAKE_BINARY_DIR})

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(test)
