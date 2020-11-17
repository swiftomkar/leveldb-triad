//
// Created by Omkar Desai on 11/16/20.
//

#ifndef LEVELDB_WORKLOAD_CHARACTERIZER_H
#define LEVELDB_WORKLOAD_CHARACTERIZER_H

#include <cstddef>
#include <cstdint>

namespace leveldb {
class WorkkloadType{
 public:
  WorkkloadType();

 private:
  double skew;

};
}
#endif  // LEVELDB_WORKLOAD_CHARACTERIZER_H
