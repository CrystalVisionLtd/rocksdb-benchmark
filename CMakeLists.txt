cmake_minimum_required(VERSION 3.21)
project(RocksDBBenchmark)

set(CMAKE_CXX_STANDARD 20)
set(PYBIND11_FINDPYTHON True)

add_subdirectory(deps/pybind11)
include_directories(deps/pybind11/include)

add_subdirectory(deps/rocksdb)
include_directories(deps/rocksdb/include)

pybind11_add_module(rocksbench src/pymain.cpp)
target_link_libraries(rocksbench PRIVATE rocksdb)

add_executable(diskbench src/main.cpp)
