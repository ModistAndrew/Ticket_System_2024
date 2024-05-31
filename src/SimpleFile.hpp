//
// Created by zjx on 2024/5/26.
//

#ifndef TICKETSYSTEM2024_SIMPLE_FILE_HPP
#define TICKETSYSTEM2024_SIMPLE_FILE_HPP

#include <fstream>
#include <filesystem>
#include "Exceptions.hpp"
#include "Util.hpp"

using std::string;
using std::fstream;
using std::ifstream;
using std::ofstream;

template<int CACHE_SIZE = 100000>
class SimpleFile { //you can write a string at end of file and read a string randomly
  fstream file;
  string fileName;
  struct Cache {
    std::string data;
    bool dirty = false;
  };
  map<int, Cache> cacheMap;
  int cacheSize = 0;
public:
  explicit SimpleFile(const string &file_name) : fileName("storage/" + file_name + ".dat") {
    if (!std::filesystem::exists(fileName)) {
      std::filesystem::create_directory("storage");
      file.open(fileName, std::ios::out | std::ios::binary);
      file.close();
    }
    file.open(fileName, std::ios::in | std::ios::out | std::ios::binary);
  }

  void checkCache() {
    if(cacheSize > CACHE_SIZE) {
      for(auto it = cacheMap.begin(); it != cacheMap.end(); it++) {
        if(it->second.dirty) {
          file.seekp(it->first);
          file << it->second.data;
        }
      }
      cacheMap.clear();
      cacheSize = 0;
    }
  }

  ~SimpleFile() {
    for(auto it = cacheMap.begin(); it != cacheMap.end(); it++) {
      if(it->second.dirty) {
        file.seekp(it->first);
        file << it->second.data;
      }
    }
    file.close();
  }

  int write(const std::string &str) {
    int ret = file.seekp(0, std::ios::end).tellp();
    file << str << '\n';
    return ret;
  }

  std::string *get(int pos, bool dirty) {
    auto it = cacheMap.find(pos);
    if (it == cacheMap.end()) {
      it = cacheMap.insert({pos, {}}).first;
      file.seekg(pos);
      std::getline(file, it->second.data);
      cacheSize+=it->second.data.length() + sizeof(Cache);
    }
    if (dirty) {
      it->second.dirty = true;
    }
    return &it->second.data;
  }
};


#endif
