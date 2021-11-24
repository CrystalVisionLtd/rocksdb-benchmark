//
// Created by wuyua on 11/23/2021.
//

#ifndef ROCKSBENCH_H
#define ROCKSBENCH_H
#include <rocksdb/convenience.h>
#include <rocksdb/db.h>
#include <rocksdb/iostats_context.h>
#include <rocksdb/perf_context.h>
#include <rocksdb/table.h>

#include "comparator.hpp"

using namespace rocksdb;
using namespace std::chrono;

template <typename T>
std::string pack_slice(T val) {
  std::string data(reinterpret_cast<const char*>(&val), sizeof val);
  return data;
}

class RocksBench {
 private:
  DB* db{nullptr};

 public:
  explicit RocksBench(const std::string& path, const Options& db_opt) {
    //    BlockBasedTableOptions table_options;
    //    table_options.no_block_cache = true;

    Options opts(db_opt);
    opts.IncreaseParallelism();
    opts.OptimizeLevelStyleCompaction();
    opts.create_if_missing = true;
    opts.compression = rocksdb::kNoCompression;
    opts.statistics = CreateDBStatistics();
    opts.comparator = UInt64Comparator();
    //    opts.compaction_pri = kMinOverlappingRatio;
    //    opts.table_factory.reset(NewBlockBasedTableFactory(table_options));
    opts.enable_blob_files = true;
    opts.enable_blob_garbage_collection = true;
    opts.blob_garbage_collection_force_threshold = 1.0;
    opts.blob_garbage_collection_age_cutoff = 1.0;
    opts.blob_compression_type = kNoCompression;
    opts.min_blob_size = 0;

    Status s = DB::Open(opts, path, &db);
    assert(s.ok());
  }
  virtual ~RocksBench() { delete db; }
  duration<double> BenchWrite(
      int count,
      size_t unit_size,
      uint64_t first_key = 0,
      bool disable_wal = false) {
    assert(db);
    assert(unit_size > 0);
    assert(count > 0);
    std::string data(unit_size, 0x01);
    WriteOptions wopts;
    wopts.disableWAL = disable_wal;
    Status s;
    auto start = high_resolution_clock::now();
    for (int i = 0; i < count; ++i) {
      s = db->Put(wopts, pack_slice(i + first_key), data);
      assert(s.ok());
    }
    auto end = high_resolution_clock::now();

    return end - start;
  }

  duration<double> BenchFlush() {
    assert(db);
    FlushOptions fopts;
    auto start = high_resolution_clock::now();
    auto s = db->Flush(fopts);
    auto end = high_resolution_clock::now();

    assert(s.ok());
    return end - start;
  }

  duration<double> BenchRead(uint64_t key) {
    assert(db);
    ReadOptions ropts;
    std::string data;
    auto start = high_resolution_clock::now();
    auto s = db->Get(ropts, pack_slice(key), &data);
    auto end = high_resolution_clock::now();

    assert(s.ok());
    return end - start;
  }

  duration<double> BenchDeleteRange(uint64_t begin_key, uint64_t end_key) {
    assert(db);
    WriteOptions wopts;
    auto start = high_resolution_clock::now();
    auto s = db->DeleteRange(
        wopts, nullptr, pack_slice(begin_key), pack_slice(end_key));
    auto end = high_resolution_clock::now();
    assert(s.ok());

    return end - start;
  }
  duration<double> BenchReduceByFactor(
      int factor,
      uint64_t begin_key,
      uint64_t end_key) {
    assert(db);
    assert(end_key > begin_key);
    size_t n_blocks = (end_key - begin_key) / factor;
    WriteOptions wopts;

    auto start = high_resolution_clock::now();
    for (int i = 0; i < n_blocks; ++i) {
      auto s = db->DeleteRange(
          wopts,
          nullptr,
          pack_slice(begin_key + 1),
          pack_slice(begin_key + factor + 1));
      assert(s.ok());
      begin_key += factor;
    }
    auto end = high_resolution_clock::now();
    return end - start;
  }

  duration<double> BenchCompact() {
    assert(db);
    CompactRangeOptions c_opts;
    auto start = high_resolution_clock::now();
    auto s = db->CompactRange(c_opts, nullptr, nullptr);
    auto end = high_resolution_clock::now();

    assert(s.ok());

    return end - start;
  }

  duration<double> BenchIterator() {
    assert(db);
    ReadOptions ropts;
    ropts.iterate_upper_bound = nullptr;

    auto start = high_resolution_clock::now();
    auto it = db->NewIterator(ropts);
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
      auto value = it->value();
      auto key = it->key();
      assert(!value.empty());
      assert(!key.empty());
    }
    assert(it->status().ok());
    delete it;
    auto end = high_resolution_clock::now();
    return end - start;
  }

  std::string GetProperty(const std::string& key) {
    assert(db);
    std::string value;
    auto s = db->GetProperty(key, &value);
    assert(s);
    return value;
  }

  std::unordered_map<std::string, std::string> GetBlobProperties() {
    assert(db);
    std::string keys[] = {
        "rocksdb.num-blob-files",
        "rocksdb.blob-stats",
        "rocksdb.total-blob-file-size",
        "rocksdb.live-blob-file-size",
        "rocksdb.estimate-live-data-size",
    };
    std::unordered_map<std::string, std::string> ret;
    for (const auto& k : keys) {
      std::string value;
      assert(db->GetProperty(k, &value));
      ret[k] = value;
    }

    return ret;
  }

  /// See
  /// https://github.com/facebook/rocksdb/wiki/RocksDB-Tuning-Guide#parallelism-options
  /// \param num_threads
  /// \param priority
  void SetBackgroundThreads(int num_threads, Env::Priority priority) {
    db->GetOptions().env->SetBackgroundThreads(num_threads, priority);
  }

  static void BeginPerf(const PerfLevel level = kDisable) {
    SetPerfLevel(level);
    get_perf_context()->Reset();
    get_iostats_context()->Reset();
  }

  static void EndPerf() { SetPerfLevel(rocksdb::PerfLevel::kDisable); }

  static std::string GetIOStatsContext() {
    return get_iostats_context()->ToString();
  }

  static std::string GetPerfContext() { return get_perf_context()->ToString(); }

  std::string Statistics() { return db->GetOptions().statistics->ToString(); }
};

#endif  // ROCKSBENCH_H
