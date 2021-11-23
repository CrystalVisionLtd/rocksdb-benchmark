//
// Created by wuyua on 11/22/2021.
//

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <pybind11/chrono.h>

#include "bench.hpp"
#include "rocksbench.hpp"

#define BREAKREF(cls, var) #var, &cls::var
#define BREAK(cls, var) #var, cls::var
#define DEF_RW(cls, var) def_readwrite(BREAKREF(cls, var))
#define VALUE(cls, var) value(BREAK(cls, var))
#define FNAME(cls, fname) #fname, &cls::fname
namespace py = pybind11;
using namespace pybind11::literals;

PYBIND11_MODULE(rocksbench, m) {
  py::enum_<PerfLevel>(m, "PerfLevel")
      .VALUE(PerfLevel, kDisable)
      .VALUE(PerfLevel, kEnableCount)
      .VALUE(PerfLevel, kEnableTimeExceptForMutex)
      .VALUE(PerfLevel, kEnableTimeAndCPUTimeExceptForMutex)
      .VALUE(PerfLevel, kEnableTime);

  py::enum_<Env::Priority>(m, "Priority")
      .VALUE(Env::Priority, BOTTOM)
      .VALUE(Env::Priority, LOW)
      .VALUE(Env::Priority, HIGH)
      .VALUE(Env::Priority, USER)
      .VALUE(Env::Priority, TOTAL);

  py::class_<DiskPerf>(m, "DiskPerf")
      .def(py::init())
      .def_readonly("read_cont_bps", &DiskPerf::read_cont_bps)
      .def_readonly("read_rand_bps", &DiskPerf::read_rand_bps)
      .def_readonly("write_cont_bps", &DiskPerf::write_cont_bps)
      .def_readonly("write_rand_bps", &DiskPerf::write_rand_bps);
  m.def(
      "bench_disk_performance",
      &bench_disk_performance,
      "path"_a,
      "total_bytes"_a = 1ULL << 30,
      "unit_size"_a = 4ULL << 10);

  m.def("human_readable", &human_readable, "bps"_a);

  py::class_<Options>(m, "Options")
      .def(py::init())
      // Write buffer size
      // See
      // https://github.com/facebook/rocksdb/wiki/RocksDB-Tuning-Guide#parallelism-options
      .DEF_RW(Options, level_compaction_dynamic_level_bytes)
      .DEF_RW(Options, max_background_compactions)
      .DEF_RW(Options, max_background_jobs)
      .DEF_RW(Options, disable_auto_compactions)
      .DEF_RW(Options, unordered_write)
      .DEF_RW(Options, use_direct_io_for_flush_and_compaction)
      .DEF_RW(Options, use_direct_reads)
      .DEF_RW(Options, write_buffer_size)
      // consider setting this to a multiple (e.g. 8x or 10x) of
      // target_file_size_base
      .DEF_RW(Options, max_bytes_for_level_base)
      // One guideline is to set blob_file_size to the same value as
      // write_buffer_size (adjusted for compression if needed) and make
      // target_file_size_base proportionally smaller based on the ratio of key
      // size to value size.
      .DEF_RW(Options, target_file_size_base)
      .DEF_RW(Options, blob_file_size)
      .DEF_RW(Options, compaction_readahead_size)
      .DEF_RW(Options, writable_file_max_buffer_size);

  // clang-format off
  py::class_<RocksBench>(m, "RocksBench")
      .def(py::init<const std::string&, const Options&>(), "path"_a, "options"_a)
      .def(FNAME(RocksBench, BenchWrite), "count"_a, "unit_size"_a, "first_key"_a = 0)
      .def(FNAME(RocksBench, BenchFlush))
      .def(FNAME(RocksBench, BenchReduceByFactor), "factor"_a, "begin_key"_a, "end_key"_a)
      .def(FNAME(RocksBench, BenchCompact))
      .def(FNAME(RocksBench, BenchRead), "key"_a)
      .def(FNAME(RocksBench, BenchIterator))
      .def(FNAME(RocksBench, DestoryDB))
      .def(FNAME(RocksBench, GetProperty), "key"_a)
      .def(FNAME(RocksBench, GetBlobProperties))
      .def(FNAME(RocksBench, SetBackgroundThreads), "num_threads"_a, "priority"_a)
      .def_static(FNAME(RocksBench, BeginPerf), "level"_a = kDisable)
      .def_static(FNAME(RocksBench, EndPerf))
      .def(FNAME(RocksBench, GetIOStatsContext))
      .def(FNAME(RocksBench, GetPerfContext))
      .def(FNAME(RocksBench, Statistics));
  // clang-format on
}