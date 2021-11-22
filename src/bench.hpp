//
// Created by wuyua on 11/22/2021.
//

#ifndef BENCH_HPP
#define BENCH_HPP
#include <chrono>
#include <format>
#include <fstream>
#include <random>

using namespace std::chrono;

struct DiskPerf {
  double read_cont_bps;
  double read_rand_bps;
  double write_cont_bps;
  double write_rand_bps;
};


DiskPerf bench_disk_performance(
    const std::string& path,
    size_t total_bytes = 1ULL << 30,
    size_t unit_size = 4ULL << 10) {
  DiskPerf perf{};

  std::remove(path.c_str());

  std::ofstream fout;
  fout.open(path, std::ios_base::out | std::ios_base::binary);
  std::string buf;
  buf.resize(unit_size);

  std::vector<size_t> blocks;
  size_t n_blocks = total_bytes / unit_size;
  total_bytes = n_blocks * unit_size;
  blocks.resize(n_blocks);
  for (int i = 0; i < n_blocks; ++i) {
    blocks[i] = i * unit_size;
  }
  auto rd = std::random_device{};
  auto rng = std::default_random_engine{rd()};
  std::shuffle(blocks.begin(), blocks.end(), rng);

  // Continuous write
  auto start = high_resolution_clock::now();
  for (int i = 0; i < n_blocks; ++i) {
    fout.write(buf.c_str(), buf.size());
  }
  auto stop = high_resolution_clock::now();
  duration<double> duration = stop - start;
  double write_cont_bps = static_cast<double>(total_bytes) / duration.count();
  perf.write_cont_bps = write_cont_bps;

  // Random write
  start = high_resolution_clock::now();
  for (auto offset : blocks) {
    fout.seekp(offset);
    fout.write(buf.c_str(), buf.size());
  }
  stop = high_resolution_clock::now();
  duration = stop - start;
  double write_rand_bps = static_cast<double>(total_bytes) / duration.count();
  perf.write_rand_bps = write_rand_bps;

  fout.close();

  // Continuous read
  std::ifstream fin(path, std::ios_base::binary | std::ios_base::in);

  start = high_resolution_clock::now();
  for (int i = 0; i < n_blocks; ++i) {
    fin.read(buf.data(), buf.size());
  }
  stop = high_resolution_clock::now();
  duration = stop - start;
  double read_cont_bps = static_cast<double>(total_bytes) / duration.count();
  perf.read_cont_bps = read_cont_bps;

  // Random read
  start = high_resolution_clock::now();
  for (auto offset : blocks) {
    fin.seekg(offset);
    fin.read(buf.data(), buf.size());
  }
  stop = high_resolution_clock::now();
  duration = stop - start;
  double read_rand_bps = static_cast<double>(total_bytes) / duration.count();
  perf.read_rand_bps = read_rand_bps;
  fin.close();

  std::remove(path.c_str());
  return perf;
}

std::string human_readable(double bps) {
  const std::string units[] = {"bytes/s", "kb/s", "mb/s", "gb/s", "tb/s"};
  int idx = 0;
  while (bps > 1024) {
    bps /= 1024;
    idx += 1;
  }
  std::stringstream stream;
  stream << std::fixed << std::setprecision(2) << bps;
  std::string s = stream.str();
  return s + " " + units[idx];
}

#endif  // BENCH_HPP
