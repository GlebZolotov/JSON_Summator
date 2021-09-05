cmake_minimum_required (VERSION 2.6)

project (JSON_Summator)

set(SOURCE_EXE JSON_Summator.cpp logging.cpp manager.cpp worker_thread.cpp worker.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(CppKafka REQUIRED)
find_package(Boost COMPONENTS program_options log_setup log REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lboost_thread -lpthread")

add_executable(main ${SOURCE_EXE})	# Создает исполняемый файл с именем main
target_link_libraries(main PRIVATE Threads::Threads CppKafka::cppkafka ${Boost_LIBRARIES} boost_thread)
