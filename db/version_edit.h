// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_LEVELDB_DB_VERSION_EDIT_H_
#define STORAGE_LEVELDB_DB_VERSION_EDIT_H_

#include <set>
#include <utility>
#include <vector>
#include <memory>

#include "db/dbformat.h"
#include "util/hyperloglog.h"
#include "util/murmurhash.h"

namespace leveldb {

class VersionSet;

struct FileMetaData {
  FileMetaData() : refs(0),
                   allowed_seeks(1 << 30),
                   file_size(0),
                   //OMKAR
                   reclaim_ratio(0.0),
                   hll_add_count(0),
                   hll(std::make_shared<HyperLogLog>(12)),
                   num_sst_next_level_overlap(-1),
                   file_num_low(-1),//(ULLONG_MAX),
                   file_num_high(-1){}
                   //OMKAR

  int refs;
  int allowed_seeks;  // Seeks allowed until compaction
  uint64_t number;
  uint64_t file_size;    // File size in bytes
  InternalKey smallest;  // Smallest internal key served by table
  InternalKey largest;   // Largest internal key served by table
  //OMKAR
  std::shared_ptr<HyperLogLog> hll;
  int hll_add_count;
  double reclaim_ratio;
  //Maybe useful stuff
  uint64_t file_num_low;
  uint64_t file_num_high;
  int num_sst_next_level_overlap;

  void updateFileMetaData(const Slice& key){
    const Slice& user_key = ExtractUserKey(key);
    int64_t hash = MurmurHash64A(user_key.data(), user_key.size(), 0);
    hll->AddHash(hash);
    ++hll_add_count;
  }
  //OMKAR
};

class VersionEdit {
 public:
  VersionEdit() { Clear(); }
  ~VersionEdit() = default;

  void Clear();

  void SetComparatorName(const Slice& name) {
    has_comparator_ = true;
    comparator_ = name.ToString();
  }
  void SetLogNumber(uint64_t num) {
    has_log_number_ = true;
    log_number_ = num;
  }
  void SetPrevLogNumber(uint64_t num) {
    has_prev_log_number_ = true;
    prev_log_number_ = num;
  }
  void SetNextFile(uint64_t num) {
    has_next_file_number_ = true;
    next_file_number_ = num;
  }
  void SetLastSequence(SequenceNumber seq) {
    has_last_sequence_ = true;
    last_sequence_ = seq;
  }
  void SetCompactPointer(int level, const InternalKey& key) {
    compact_pointers_.push_back(std::make_pair(level, key));
  }

  // Add the specified file at the specified number.
  // REQUIRES: This version has not been saved (see VersionSet::SaveTo)
  // REQUIRES: "smallest" and "largest" are smallest and largest keys in file
  void AddFile(int level, uint64_t file, uint64_t file_size,
               const InternalKey& smallest, const InternalKey& largest,
               //OMKAR
               const std::shared_ptr<HyperLogLog>& hll=NULL,
               double reclaim_ratio=0,
               int hll_add_count=0,

               int num_sst_next_level_overlap=0,
               uint64_t file_num_low=0,
               uint64_t file_num_high=0
               //OMKAR
               ) {
    FileMetaData f;
    f.number = file;
    f.file_size = file_size;
    f.smallest = smallest;
    f.largest = largest;

    //OMKAR
    f.hll = hll;
    f.hll_add_count = hll_add_count;
    f.reclaim_ratio = reclaim_ratio;

    f.file_num_low = file_num_low;
    f.file_num_high = file_num_high;
    f.num_sst_next_level_overlap = num_sst_next_level_overlap;

    //OMKAR
    new_files_.emplace_back(level, f);
  }

  // Delete the specified "file" from the specified "level".
  void RemoveFile(int level, uint64_t file) {
    deleted_files_.insert(std::make_pair(level, file));
  }

  void EncodeTo(std::string* dst) const;
  Status DecodeFrom(const Slice& src);

  std::string DebugString() const;

 private:
  friend class VersionSet;

  typedef std::set<std::pair<int, uint64_t>> DeletedFileSet;

  std::string comparator_;
  uint64_t log_number_;
  uint64_t prev_log_number_;
  uint64_t next_file_number_;
  SequenceNumber last_sequence_;
  bool has_comparator_;
  bool has_log_number_;
  bool has_prev_log_number_;
  bool has_next_file_number_;
  bool has_last_sequence_;

  std::vector<std::pair<int, InternalKey>> compact_pointers_;
  DeletedFileSet deleted_files_;
  std::vector<std::pair<int, FileMetaData>> new_files_;
};

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_DB_VERSION_EDIT_H_
