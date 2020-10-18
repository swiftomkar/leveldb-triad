#include <cassert>
#include "leveldb/db.h"
#include <iostream>

int main(int argc, char* argv[]) {
  std::cout<<"Welcome to Omkar's simple leveldb tester\n";
  leveldb::DB *db;
  leveldb::Options options;
  options.create_if_missing = true;
  leveldb::Status status = leveldb::DB::Open(options, "/tmp/testdb", &db);
  std::string value, key, return_value;
  leveldb::Status s;
  for(int i = 0; i<400000;i++) {
    value = "value" + std::to_string(i);
    std::string ret_value;
    key = std::to_string(i);
    s = db->Put(leveldb::WriteOptions(), key, value);
    s = db->Put(leveldb::WriteOptions(), key, value+".v2");
    s = db->Get(leveldb::ReadOptions(), key, &return_value);
    //std::cout << return_value << "\n";
  }
  leveldb::Iterator* db_iter = db->NewIterator(leveldb::ReadOptions());
  s=db->Get(leveldb::ReadOptions(), "4", &value);
  db_iter->SeekToFirst();
  while(db_iter->Valid()){
    leveldb::Slice value_ = db_iter->value();
    leveldb::Slice key_ = db_iter->key();
    //std::cout << key_.ToString()<<" <--> " << value_.ToString() << "\n";
    db_iter->Next();
  }
  //leveldb::Slice slice = db_iter->value();
  //std::cout << slice.ToString() << "\n";
  /*
  std::string property;
  db->GetProperty("leveldb.stats", &property);
  std::cout << property << "\n";
  db->GetProperty("leveldb.approximate-memory-usage", &property);
  std::cout << property << "\n";
  db->GetProperty("leveldb.num-files-at-level5", &property);
  std::cout << property << "\n";
  db->GetProperty("leveldb.sstables", &property);
  std::cout << property << "\n";
   */
  assert(status.ok());
  delete db;

}
