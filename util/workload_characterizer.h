//
// Created by Omkar Desai on 11/16/20.
//

#ifndef LEVELDB_WORKLOAD_CHARACTERIZER_H
#define LEVELDB_WORKLOAD_CHARACTERIZER_H

#include <iostream>
#include <cstddef>
#include <cstdint>

namespace leveldb {

class WorkloadType{

 public:
  WorkloadType()=default;

  static void getCountInc();

  static void putCountInc();

  static double getOverlapRatio();

  static void getWorkloadstat();

 private:
  //double skew;
  static double readWriteRatio;
  static int windowCount;
  static double getCount;
  static double putCount;
};
}
#endif  // LEVELDB_WORKLOAD_CHARACTERIZER_H
