//
// Created by Omkar Desai on 11/16/20.
//
#include "util/workload_characterizer.h"

namespace leveldb{
double WorkloadType::readWriteRatio = 1.0;
int WorkloadType::windowCount = 0;
double WorkloadType::getCount= 1.0;
double WorkloadType::putCount = 1.0;

//WorkloadType::WorkloadType() {
//  //skew = 0.0;
//  readWriteRatio = 0.0;
//  getCount = 0.0;
//  putCount = 0.0;
//  windowCount = 0;
//}

void WorkloadType::getCountInc() {
  if (windowCount == 10000){
    windowCount = 0;
    getCount = 0;
  }
  windowCount++;
  getCount++;
}

void WorkloadType::putCountInc() {
  if (windowCount == 10000){
    windowCount = 0;
    putCount = 0;
  }
  windowCount++;
  putCount++;
}

void WorkloadType::getWorkloadstat() {
  std::cout <<"------------------------" << "\n";
  std::cout <<"window count: " << windowCount << "\n";
  std::cout <<"get count: " << getCount << "\n";
  std::cout <<"put count: " << putCount << "\n";
  std::cout <<"ratio: " << readWriteRatio << "\n";
  std::cout <<"------------------------" << "\n";
}

double WorkloadType::getOverlapRatio() {
  readWriteRatio = getCount/putCount;
  //std::cout << "readWriteRatio: " << readWriteRatio << "\n";
  if(readWriteRatio <= 0.1){
    return 0.0;
  }
  else{
    return 0.4;
  }
}

}