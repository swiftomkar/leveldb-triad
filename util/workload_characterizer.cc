//
// Created by Omkar Desai on 11/16/20.
//
#include "util/workload_characterizer.h"

namespace leveldb{

void WorkloadType::getCountInc() {
  if (windowCount == 10000){
    windowCount = 0;
    getCount = 0;
  }
  windowCount++;
  getCount++;
  readWriteRatio = getCount/putCount;
}

void WorkloadType::putCountInc() {
  if (windowCount == 10000){
    windowCount = 0;
    putCount = 0;
  }
  windowCount++;
  putCount++;
  readWriteRatio = getCount/putCount;
}

double WorkloadType::getOverlapRatio() {
  if(readWriteRatio == 0){
    return 0.0;
  }
  else{
    return 0.4;
  }
}

}