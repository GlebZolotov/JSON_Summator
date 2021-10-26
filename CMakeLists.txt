cmake_minimum_required (VERSION 3.8)
project (JSON_Summator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_FLAGS "-Wall -g")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=corei7-avx -funroll-loops") #-finline-functions
SET(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g -Wall -Wextra -Wpedantic")

set(SOURCE_EXE JSON_Summator.cpp logging.cpp manager.cpp worker_thread.cpp worker.cpp csv_handler.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_library(P7LOGGER p7)
find_library(RT_FOR_P7 rt)
find_package(Threads REQUIRED)
find_package(CppKafka REQUIRED)
find_package(Protobuf REQUIRED)
find_package(Boost COMPONENTS program_options log_setup log REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${Protobuf_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/P7Client/Headers)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vtb-project/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vtb-project/src/milp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vtb-project/src/LP)
add_subdirectory(vtb-project)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS input_data.proto output_data.proto)

add_executable(main ${SOURCE_EXE} ${PROTO_SRCS} ${PROTO_HDRS})	# Создает исполняемый файл с именем main
target_link_libraries(main PRIVATE Threads::Threads CppKafka::cppkafka ${Boost_LIBRARIES} boost_thread ${Protobuf_LIBRARIES} ${P7LOGGER} ${RT_FOR_P7} Eigen3::Eigen mad_optimizer_lib)
