add_executable(test-static test_static.cpp)
target_link_libraries(test-static PRIVATE ordered)
add_test(static ${CMAKE_CURRENT_BINARY_DIR}/test-static)

add_executable(test-examples test_examples.cpp)
target_link_libraries(test-examples PRIVATE ordered)
add_test(examples ${CMAKE_CURRENT_BINARY_DIR}/test-examples)
