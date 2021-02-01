cmake_minimum_required(VERSION 3.16)
project(Cjudger)

set(CMAKE_CXX_STANDARD 14)

add_executable(run include/goj_env.h include/goj_judger.h run/run_program.cpp)
add_executable(compile run/compile_program.cpp)
add_executable(test run/Test.cpp)
add_executable(judge run/judge.cpp)