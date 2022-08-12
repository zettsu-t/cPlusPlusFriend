cmake_minimum_required(VERSION 3.16)
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)

project(juliaset CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -pthread")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage -pthread")

set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Use Boost C++ libraries
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)
set(Boost_USE_MULTITHREADED ON)

# Use Google Test
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" . RESULT_VARIABLE result WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)

if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()

execute_process(COMMAND ${CMAKE_COMMAND} --build . RESULT_VARIABLE result WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)

if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src ${CMAKE_CURRENT_BINARY_DIR}/googletest-build EXCLUDE_FROM_ALL)

enable_testing()
include(GoogleTest)

# Include header files
set(BASEPATH "${CMAKE_SOURCE_DIR}")

# Executable unit tests
add_executable(juliaset juliaset_impl.cpp juliaset.cpp)
add_executable(test_juliaset juliaset_impl.cpp test_juliaset.cpp)
add_custom_target(run COMMAND juliaset DEPENDS juliaset WORKING_DIRECTORY ${CMAKE_PROJECT_DIR})
target_compile_options(test_juliaset PRIVATE -Wall -Wextra -Wconversion -Wformat=2 -Wcast-qual -Wcast-align -Wwrite-strings -Wfloat-equal -Wpointer-arith -Wno-unused-parameter -DUNIT_TEST)
target_link_libraries(juliaset boost_program_options png Threads::Threads)
target_link_libraries(test_juliaset boost_program_options gtest_main png Threads::Threads)
gtest_add_tests(TARGET test_juliaset)
