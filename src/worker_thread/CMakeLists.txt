#-----BEGIN OF Sources List-----
set(SOURCES_WORKER_THR ${SOURCES_WORKER_THR} worker_thread.cpp)

#-----END OF Sources List-----

add_library(worker_thread_lib ${SOURCES_WORKER_THR})
target_include_directories(worker_thread_lib PUBLIC ./ ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(worker_thread_lib PUBLIC logging_lib worker_lib utils_lib)