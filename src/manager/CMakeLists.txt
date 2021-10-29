#-----BEGIN OF Sources List-----
set(SOURCES_MANAGER ${SOURCES_MANAGER} manager.cpp)

#-----END OF Sources List-----

add_library(manager_lib ${SOURCES_MANAGER})
target_include_directories(manager_lib PUBLIC ./ ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(manager_lib PUBLIC logging_lib worker_lib)