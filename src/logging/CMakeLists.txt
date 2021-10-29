#-----BEGIN OF Sources List-----
set(SOURCES_LOGGING ${SOURCES_LOGGING} logging.cpp)

#-----END OF Sources List-----

find_library(P7LOGGER p7)
find_library(RT_FOR_P7 rt)

add_library(logging_lib ${SOURCES_LOGGING})
target_include_directories(logging_lib PUBLIC ./ ${PROJECT_SOURCE_DIR}/include/P7_Headers)
target_link_libraries(logging_lib PUBLIC p7 rt)