//
// Created by Omkar Desai on 11/16/20.
//

#ifndef LEVELDB_WORKLOAD_CHARACTERIZER_H
#define LEVELDB_WORKLOAD_CHARACTERIZER_H

#include <cstddef>
#include <cstdint>

namespace leveldb {
class WorkloadType{
 public:
  WorkloadType() = default;

  void getCountInc();

  void putCountInc();

  double getOverlapRatio();

 private:
  double skew;
  double readWriteRatio;
  int windowCount;
  double getCount;
  double putCount;


};
}
#endif  // LEVELDB_WORKLOAD_CHARACTERIZER_H
