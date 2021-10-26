#-----BEGIN OF Sources List-----
set(SOURCES ${SOURCES} ./milp/mad_max/advance_tableu.cpp)
set(SOURCES ${SOURCES} ./milp/mad_max/node.cpp)
set(SOURCES ${SOURCES} ./milp/mad_max/best_solution.cpp)
set(SOURCES ${SOURCES} ./milp/mad_max/MADmaximize.cpp)
set(SOURCES ${SOURCES} ./milp/mad_max/MADmax_integer_local_improve.cpp)
set(SOURCES ${SOURCES} ./milp/mad_max/quick_return_to_feasibility.cpp)
set(SOURCES ${SOURCES} ./milp/mad_max/return_table_to_feasibility.cpp)
set(SOURCES ${SOURCES} ./io/csv_reader.cpp)
set(SOURCES ${SOURCES} params.cpp)

set(SOURCES ${SOURCES} ./LP/simplex_method.cpp)
set(SOURCES ${SOURCES} solver.hpp)
set(SOURCES ${SOURCES} ./LP/lp_solver.cpp)
set(SOURCES ${SOURCES} ./LP/mad_min_lp_solver.cpp)
set(SOURCES ${SOURCES} ./LP/mad_max_lp_solver.cpp)
set(SOURCES ${SOURCES} types.hpp)
set(SOURCES ${SOURCES} utils.cpp)

set(SOURCES ${SOURCES} task_former/task_former.cpp)
set(SOURCES ${SOURCES} ./milp/MADmax_solver.cpp)
#-----END OF Sources List-----

add_subdirectory(./milp/mad_min/)

add_library(mad_optimizer_lib ${SOURCES})
target_include_directories(mad_optimizer_lib PUBLIC ./ ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(mad_optimizer_lib PUBLIC Eigen3::Eigen json_csv_lib madmin)

add_executable (simplex_demo ./runner/simplex_demo.cpp)
target_include_directories(simplex_demo PRIVATE ./)
target_link_libraries(simplex_demo PUBLIC mad_optimizer_lib)

add_executable (mad_min_lp_demo ./runner/mad_min_lp_demo.cpp)
target_include_directories(mad_min_lp_demo PRIVATE ./)
target_link_libraries(mad_min_lp_demo PUBLIC mad_optimizer_lib)

add_executable (mad_max_lp_demo ./runner/mad_max_lp_demo.cpp)
target_include_directories(mad_max_lp_demo PRIVATE ./)
target_link_libraries(mad_max_lp_demo PUBLIC mad_optimizer_lib)

add_executable (vtb_optimizer ./runner/main.cpp ${SOURCES})
target_include_directories(vtb_optimizer PRIVATE ./)
target_link_libraries(vtb_optimizer PUBLIC mad_optimizer_lib)

add_executable (mad_max_test ./runner/optimizeMADmax.cpp)
target_include_directories(mad_max_test PRIVATE ./)
target_link_libraries(mad_max_test PUBLIC mad_optimizer_lib)
