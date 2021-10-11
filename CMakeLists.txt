cmake_minimum_required (VERSION 2.6)
set(CMAKE_CXX_STANDARD 17)
project (JSON_Summator)

set(SOURCE_EXE JSON_Summator.cpp logging.cpp manager.cpp worker_thread.cpp worker.cpp csv_handler.cpp dailydata.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(CppKafka REQUIRED)
find_package(Protobuf REQUIRED)
find_package(Boost COMPONENTS program_options log_setup log REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${Protobuf_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS input_data.proto output_data.proto)

add_executable(main ${SOURCE_EXE} ${PROTO_SRCS} ${PROTO_HDRS})	# Создает исполняемый файл с именем main
target_link_libraries(main PRIVATE Threads::Threads CppKafka::cppkafka ${Boost_LIBRARIES} boost_thread ${Protobuf_LIBRARIES})
