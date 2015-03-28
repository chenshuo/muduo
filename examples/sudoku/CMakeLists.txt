add_executable(sudoku_solver_basic server_basic.cc sudoku.cc)
target_link_libraries(sudoku_solver_basic muduo_net)

add_executable(sudoku_solver_multiloop server_multiloop.cc sudoku.cc)
target_link_libraries(sudoku_solver_multiloop muduo_net)

add_executable(sudoku_solver_threadpool server_threadpool.cc sudoku.cc)
target_link_libraries(sudoku_solver_threadpool muduo_net)

add_executable(sudoku_solver_hybrid server_hybrid.cc sudoku.cc)
target_link_libraries(sudoku_solver_hybrid muduo_inspect)

add_executable(sudoku_client client.cc sudoku.cc)
target_link_libraries(sudoku_client muduo_net)

if(BOOSTTEST_LIBRARY)
add_executable(sudoku_stat_unittest stat_unittest.cc)
target_link_libraries(sudoku_stat_unittest muduo_base boost_unit_test_framework)
endif()

