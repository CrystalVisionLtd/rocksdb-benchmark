//
// Created by wuyua on 11/22/2021.
//

#include <pybind11/pybind11.h>
#include <rocksdb/db.h>

#include "bench.hpp"

namespace py = pybind11;
using namespace pybind11::literals;
using namespace rocksdb;

class RocksBench {
 private:
  DB* db;

 public:
  explicit RocksBench(
      const std::string& path,
      bool use_direct_io_for_flush_and_compaction = false,
      bool allow_mmap_writes = false,
      bool allow_fallocate = true) {
    DBOptions db_opt;
    db_opt.create_if_missing = true;
    db_opt.use_direct_io_for_flush_and_compaction =
        use_direct_io_for_flush_and_compaction;
    db_opt.allow_mmap_writes = allow_mmap_writes;
    db_opt.allow_fallocate = allow_fallocate;
  }
};

PYBIND11_MODULE(rocksbench, m) {
  py::class_<DiskPerf>(m, "DiskPerf")
      .def_readonly("read_cont_bps", &DiskPerf::read_cont_bps)
      .def_readonly("read_rand_bps", &DiskPerf::read_rand_bps)
      .def_readonly("write_cont_bps", &DiskPerf::write_cont_bps)
      .def_readonly("write_rand_bps", &DiskPerf::write_rand_bps);
  m.def(
      "bench_disk_performance",
      &bench_disk_performance,
      "path"_a,
      "total_bytes"_a = 1ULL << 30,
      "uint_size"_a = 4ULL << 10);

  m.def("human_readable", &human_readable, "bps"_a);
}