#-----BEGIN OF Sources List-----
set(SOURCES_UTILS ${SOURCES_UTILS} loc_utils.cpp)

#-----END OF Sources List-----

add_library(utils_lib ${SOURCES_UTILS})
target_include_directories(utils_lib PUBLIC ./)