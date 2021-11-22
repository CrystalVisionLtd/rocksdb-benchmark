//
// Created by wuyua on 11/22/2021.
//
#include "bench.hpp"
#include <iostream>


int main() {
  auto ret = bench_disk_performance("./data");
  std::cout << "Continuous read:" << human_readable(ret.read_cont_bps) << std::endl;
  std::cout << "Random read:" << human_readable(ret.read_rand_bps) << std::endl;
  std::cout << "Continuous write:" << human_readable(ret.write_cont_bps) << std::endl;
  std::cout << "Random write:" << human_readable(ret.write_rand_bps) << std::endl;
}