//
// Created by zjx on 2024/5/26.
//

#ifndef TICKETSYSTEM2024_SIMPLE_FILE_HPP
#define TICKETSYSTEM2024_SIMPLE_FILE_HPP

#include <fstream>
#include <filesystem>
#include "../util/Exceptions.hpp"
#include "../util/Util.hpp"

using std::string;
using std::fstream;
using std::ifstream;
using std::ofstream;

//you can write a string at end of file and read a string randomly. you can modify but the data length should be the same.
//use map cache. for small cache size.
template<typename T>
class SimpleFile {
  fstream file;
  string fileName;
  struct Cache {
    T data;
    bool dirty = false;
  };
  map<int, Cache> cacheMap;
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
    for (auto it = cacheMap.begin(); it != cacheMap.end(); it++) {
      if (it->second.dirty) {
        file.seekp(it->first);
        file << it->second.data.toString();
      }
    }
    cacheMap.clear();
  }

  ~SimpleFile() {
    for (auto it = cacheMap.begin(); it != cacheMap.end(); it++) {
      if (it->second.dirty) {
        file.seekp(it->first);
        file << it->second.data.toString();
      }
    }
    file.close();
  }

  int write(const T &str) {
    int ret = file.seekp(0, std::ios::end).tellp();
    file << str.toString() << '\n';
    return ret;
  }

  T *get(int pos, bool dirty) {
    auto it = cacheMap.find(pos);
    if (it == cacheMap.end()) {
      it = cacheMap.insert({pos, {}}).first;
      file.seekg(pos);
      std::string tmp;
      std::getline(file, tmp);
      it->second.data = T(tmp);
    }
    if (dirty) {
      it->second.dirty = true;
    }
    return &it->second.data;
  }
};


#endif
