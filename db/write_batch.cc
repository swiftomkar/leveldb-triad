// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// WriteBatch::rep_ :=
//    sequence: fixed64
//    count: fixed32
//    data: record[count]
// record :=
//    kTypeValue varstring varstring         |
//    kTypeDeletion varstring
// varstring :=
//    len: varint32
//    data: uint8[len]

#include "leveldb/write_batch.h"

#include "db/dbformat.h"
#include "db/memtable.h"
#include "db/write_batch_internal.h"
#include "leveldb/db.h"
#include "util/coding.h"

namespace leveldb {

// WriteBatch header has an 8-byte sequence number followed by a 4-byte count.
static const size_t kHeader = 12;
static const uint64_t maxCounter = 10000000;
static uint64_t curCounter = 0;
static uint64_t maxWritten = 0;
static std::map<Slice, int, SliceCompare> hotcounter; // counter for differentiating cold and hot data
static bool SortHotnessComparator(std::pair<Slice, int>& a, std::pair<Slice, int>& b) { 
  return a.second > b.second; 
} 
static std::vector<std::pair<Slice, int>> HotnessSorter(std::map<Slice, int, SliceCompare> &m){
    std::vector<std::pair<Slice, int>> v;
    for (auto& it : m) { 
        v.push_back(it); 
    }
    std::sort(v.begin(), v.end(), SortHotnessComparator);
    int prev = -1;
    for (auto& it : v) { 
      assert(it.second >= prev);
      prev = it.second;
    } 
    return v;
}

WriteBatch::WriteBatch() { Clear(); }

WriteBatch::~WriteBatch() = default;

WriteBatch::Handler::~Handler() = default;

void WriteBatch::Clear() {
  rep_.clear();
  rep_.resize(kHeader);
}

size_t WriteBatch::ApproximateSize() const { return rep_.size(); }

Status WriteBatch::Iterate(Handler* handler) const {
  Slice input(rep_);
  if (input.size() < kHeader) {
    return Status::Corruption("malformed WriteBatch (too small)");
  }

  input.remove_prefix(kHeader);
  Slice key, value;
  int found = 0;
  while (!input.empty()) {
    found++;
    char tag = input[0];
    input.remove_prefix(1);
    switch (tag) {
      case kTypeValue:
        if (GetLengthPrefixedSlice(&input, &key) &&
            GetLengthPrefixedSlice(&input, &value)) {
          handler->Put(key, value);
        } else {
          return Status::Corruption("bad WriteBatch Put");
        }
        break;
      case kTypeDeletion:
        if (GetLengthPrefixedSlice(&input, &key)) {
          handler->Delete(key);
        } else {
          return Status::Corruption("bad WriteBatch Delete");
        }
        break;
      default:
        return Status::Corruption("unknown WriteBatch tag");
    }
  }
  if (found != WriteBatchInternal::Count(this)) {
    return Status::Corruption("WriteBatch has wrong count");
  } else {
    return Status::OK();
  }
}

int WriteBatchInternal::Count(const WriteBatch* b) {
  return DecodeFixed32(b->rep_.data() + 8);
}

void WriteBatchInternal::SetCount(WriteBatch* b, int n) {
  EncodeFixed32(&b->rep_[8], n);
}

SequenceNumber WriteBatchInternal::Sequence(const WriteBatch* b) {
  return SequenceNumber(DecodeFixed64(b->rep_.data()));
}

void WriteBatchInternal::SetSequence(WriteBatch* b, SequenceNumber seq) {
  EncodeFixed64(&b->rep_[0], seq);
}

void WriteBatch::Put(const Slice& key, const Slice& value) {
  WriteBatchInternal::SetCount(this, WriteBatchInternal::Count(this) + 1);
  rep_.push_back(static_cast<char>(kTypeValue));
  PutLengthPrefixedSlice(&rep_, key);
  PutLengthPrefixedSlice(&rep_, value);
}

void WriteBatch::Delete(const Slice& key) {
  WriteBatchInternal::SetCount(this, WriteBatchInternal::Count(this) + 1);
  rep_.push_back(static_cast<char>(kTypeDeletion));
  PutLengthPrefixedSlice(&rep_, key);
}

void WriteBatch::Append(const WriteBatch& source) {
  WriteBatchInternal::Append(this, &source);
}

namespace {
class MemTableInserter : public WriteBatch::Handler {
 public:
  //https://www.geeksforgeeks.org/sorting-a-map-by-value-in-c-stl/
  
  SequenceNumber sequence_;
  MemTable* mem_; //default mem table
  MemTable* hot_;
  const float hotPercentage = 0.2;
  bool KeyIsHot(const Slice& key){
    static bool result = false;
    if (hot_ == nullptr){
      return false;
    }else{
      curCounter++;
      //algorithm:
      //First calculate the average write number of each key
      /*
      static int avg = curCounter / hotcounter.size();
      maxWritten = maxWritten >= hotcounter[key] ? maxWritten : hotcounter[key];
      //then get the 3/4 quartile between the max accessed time for any key and the avg (near avg)
      const int shouldGreaterThan = maxWritten - ((maxWritten - avg) >> 2);
      if (hotcounter[key] >= shouldGreaterThan){
        result = true;
      }else{
        result = false;
      }*/
      
      std::vector<std::pair<Slice, int>> sortedHotness = HotnessSorter(hotcounter);
      int maxCount = sortedHotness.size() >> 2;
      int count = 0;
      for (std::vector<std::pair<Slice, int>>::iterator it = sortedHotness.begin() ; it != sortedHotness.end(); ++it){
        if (count >= maxCount) break;
        if (it->first == key) {
          result = true;
          break;
        }
        count++;
      }
      if (curCounter == maxCounter){
        maxWritten = 0;
        curCounter = 0;
        hotcounter.clear();
      }
      return result;
    }
  }
  void Put(const Slice& key, const Slice& value) override {
    //https://stackoverflow.com/a/4527747/3175584
    hotcounter[key]++;
    if (KeyIsHot(key)){
      hot_->Add(sequence_, kTypeValue, key, value);
    }else{
      mem_->Add(sequence_, kTypeValue, key, value);
    }
    sequence_++;
    
  }
  void Delete(const Slice& key) override {
    hotcounter[key]++;
    if (KeyIsHot(key)){
      hot_->Add(sequence_, kTypeDeletion, key, Slice());
    }else{
      mem_->Add(sequence_, kTypeDeletion, key, Slice());
    }
    sequence_++;
  }
};
}  // namespace

Status WriteBatchInternal::InsertInto(const WriteBatch* b, MemTable* memtable) {
  MemTableInserter inserter;
  inserter.sequence_ = WriteBatchInternal::Sequence(b);
  inserter.mem_ = memtable;
  return b->Iterate(&inserter);
}

Status WriteBatchInternal::InsertInto(const WriteBatch* b, MemTable* memtable, MemTable* hot) {
  MemTableInserter inserter;
  inserter.sequence_ = WriteBatchInternal::Sequence(b);
  inserter.mem_ = memtable;
  inserter.hot_ = hot;
  return b->Iterate(&inserter);
}

void WriteBatchInternal::SetContents(WriteBatch* b, const Slice& contents) {
  assert(contents.size() >= kHeader);
  b->rep_.assign(contents.data(), contents.size());
}

void WriteBatchInternal::Append(WriteBatch* dst, const WriteBatch* src) {
  SetCount(dst, Count(dst) + Count(src));
  assert(src->rep_.size() >= kHeader);
  dst->rep_.append(src->rep_.data() + kHeader, src->rep_.size() - kHeader);
}

}  // namespace leveldb
