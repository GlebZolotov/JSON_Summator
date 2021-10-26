#-----BEGIN OF Sources List-----
include_directories (../src)
#-----END OF Sources List-----

 
add_executable (test_maximize test_simplex.cpp)
#target_include_directories(test_maximize PRIVATE ../src)
target_link_libraries(test_maximize PRIVATE mad_optimizer_lib)