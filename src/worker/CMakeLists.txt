#-----BEGIN OF Sources List-----
set(SOURCES_WORKER ${SOURCES_WORKER} worker.cpp)

#-----END OF Sources List-----

find_package(Protobuf REQUIRED)

# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libraryMIPT/)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libraryMIPT/milp)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libraryMIPT/LP)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS prototypes/input_data.proto prototypes/output_data.proto)
set(SOURCES_WORKER ${SOURCES_WORKER} ${PROTO_SRCS} ${PROTO_HDRS})

add_library(worker_lib ${SOURCES_WORKER})
target_include_directories(worker_lib PUBLIC ./ ${Protobuf_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(worker_lib PUBLIC ${Protobuf_LIBRARIES} Eigen3::Eigen mad_optimizer_lib logging_lib utils_lib)