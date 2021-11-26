cmake_minimum_required(VERSION 3.16)
project(RocksDBBenchmark)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(deps/pybind11)
include_directories(deps/pybind11/include)
set(USE_RTTI 1)
add_subdirectory(deps/rocksdb)
include_directories(deps/rocksdb/include)
set_target_properties(rocksdb PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

pybind11_add_module(rocksbench src/pymain.cpp)
target_link_libraries(rocksbench PRIVATE rocksdb)

add_executable(diskbench src/main.cpp)
