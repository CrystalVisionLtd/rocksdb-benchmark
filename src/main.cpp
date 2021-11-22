//
// Created by wuyua on 11/22/2021.
//
#include <iostream>

#include "bench.hpp"
#include "cxxopts.hpp"

int main(int argc, char** argv) {
  // clang-format off
  cxxopts::Options options("diskbench", "Disk performance benchmark");
  options.add_options()
      ("p,path", "Path to test file", cxxopts::value<std::string>()->default_value("./diskbench_data"))
      ("u,unit", "unit size in bytes", cxxopts::value<size_t>())
      ("t,total", "total bytes to test", cxxopts::value<size_t>())
      ("h,help", "print usage")
  ;
  // clang-format on
  auto result = options.parse(argc, argv);
  if (result.count("help")) {
    std::cout << options.help() << std::endl;
    exit(0);
  }

  size_t total_bytes =
      result["total"].count() > 0 ? result["total"].as<size_t>() : 1ULL << 30;
  size_t unit_bytes =
      result["unit"].count() > 0 ? result["unit"].as<size_t>() : 4ULL << 10;
  auto ret = bench_disk_performance(
      result["path"].as<std::string>(), total_bytes, unit_bytes);
  std::cout << "Continuous read: " << human_readable(ret.read_cont_bps)
            << std::endl;
  std::cout << "Random read: " << human_readable(ret.read_rand_bps)
            << std::endl;
  std::cout << "Continuous write: " << human_readable(ret.write_cont_bps)
            << std::endl;
  std::cout << "Random write: " << human_readable(ret.write_rand_bps)
            << std::endl;
}