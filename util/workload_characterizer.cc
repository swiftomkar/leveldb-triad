//
// Created by Omkar Desai on 11/16/20.
//
#include "util/workload_characterizer.h"

namespace leveldb{
WorkkloadType::WorkkloadType() {
  skew = 0.0;
}

void WorkkloadType::getCountInc() {
  if (windowCount == 10000){
    windowCount = 0;
    getCount = 0;
  }
  windowCount++;
  getCount++;
  readWriteRatio = getCount/putCount;
}

void WorkkloadType::putCountInc() {
  if (windowCount == 10000){
    windowCount = 0;
    putCount = 0;
  }
  windowCount++;
  putCount++;
  readWriteRatio = getCount/putCount;
}

double WorkkloadType::getOverlapRatio() {
  if(readWriteRatio == 0){
    return 0.0;
  }
  else{
    return 0.4;
  }
}

}