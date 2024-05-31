#ifndef TICKETSYSTEM2024_FILE_STORAGE_HPP
#define TICKETSYSTEM2024_FILE_STORAGE_HPP

#include <fstream>
#include <filesystem>
#include "Exceptions.hpp"

using std::string;
using std::fstream;
using std::ifstream;
using std::ofstream;

template<class T, class INFO, int CACHE_SIZE>
class FileStorage {
  struct Cache {
    T data;
    bool dirty = false;
  };
  static constexpr int T_SIZE = sizeof(T);
  static constexpr int INFO_SIZE = sizeof(INFO);
  static constexpr int INT_SIZE = sizeof(int);
  fstream file;
  string fileName;
  map<int, Cache> cacheMap;
  int empty;
  int cacheSize = 0;

  int getEmpty() {
    return empty;
  }

  void setEmpty(int x) {
    empty = x;
  }

  static int getPos(int index) {
    return (index - INFO_SIZE - INT_SIZE) / T_SIZE;
  }

  static int getIndex(int pos) {
    return pos * T_SIZE + INFO_SIZE + INT_SIZE;
  }
  //store pointer to first empty just after info len.
  //empty except end has pointer to next empty; check EOF to determine whether at end. (so don't store anything after this)
public:
  INFO info;

  FileStorage(const INFO &initInfo, const string &file_name) : fileName("storage/" + file_name + ".dat") {
    newFile(initInfo);
    file.open(fileName, std::ios::in | std::ios::out | std::ios::binary);
    file.read(reinterpret_cast<char *>(&info), INFO_SIZE);
    file.read(reinterpret_cast<char *>(&empty), INT_SIZE);
  }

  ~FileStorage() {
    file.seekp(0);
    file.write(reinterpret_cast<const char *>(&info), INFO_SIZE);
    file.write(reinterpret_cast<const char *>(&empty), INT_SIZE);
    for (auto it = cacheMap.begin(); it != cacheMap.end(); it++) {
      if (it->second.dirty) {
        file.seekp(getIndex(it->first));
        file.write(reinterpret_cast<const char *>(&it->second.data), T_SIZE);
      }
    }
    file.close();
  }

  void checkCache() {
    if (cacheSize > CACHE_SIZE) {
      for (auto it = cacheMap.begin(); it != cacheMap.end(); it++) {
        if (it->second.dirty) {
          file.seekp(getIndex(it->first));
          file.write(reinterpret_cast<const char *>(&it->second.data), T_SIZE);
        }
      }
      cacheMap.clear();
      cacheSize = 0;
    }
  }

  void newFile(const INFO &initInfo) {
    if (std::filesystem::exists(fileName)) {
      return;
    }
    std::filesystem::create_directory("storage");
    file.open(fileName, std::ios::out | std::ios::binary);
    int initEmpty = INFO_SIZE + INT_SIZE;
    file.write(reinterpret_cast<const char *>(&initInfo), INFO_SIZE);
    file.write(reinterpret_cast<const char *>(&initEmpty), INT_SIZE);
    file.close();
  }

  //find an empty place to add T and return the index of the object
  int add(const T &t) {
    int index = getEmpty();
    file.seekg(index);
    int nxt;
    if (file.peek() == EOF) {
      nxt = index + T_SIZE;
      file.clear(); //clear EOF flag
    } else {
      file.read(reinterpret_cast<char *>(&nxt), INT_SIZE);
    }
    file.seekp(index);
    file.write(reinterpret_cast<const char *>(&t), T_SIZE);
    setEmpty(nxt);
    return index;
  }

  //make sure the index is valid
  //return the object at index
  //set dirty to true if you want to store the object back to file
  //the return value is a pointer to the object in cache. Shouldn't be stored for long.
  T *get(int index, bool dirty) {
    int pos = getPos(index);
    auto it = cacheMap.find(pos);
    if (it == cacheMap.end()) {
      it = cacheMap.insert({pos, {}}).first;
      file.seekg(getIndex(pos));
      file.read(reinterpret_cast<char *>(&it->second.data), T_SIZE);
      cacheSize += sizeof(Cache);
    }
    auto &ret = cacheMap[pos];
    if (dirty) {
      ret.dirty = true;
    }
    return &ret.data;
  }

  //make sure the index is valid
  //delete a currently occupied index
  //you should never remove an empty index!
  void remove(int index) {
    int pos = getPos(index);
    auto it = cacheMap.find(pos);
    if(it != cacheMap.end()) {
      cacheMap.erase(it);
      cacheSize -= sizeof(Cache);
    }
    int nxt = getEmpty();
    setEmpty(index);
    file.seekp(index);
    file.write(reinterpret_cast<const char *>(&nxt), INT_SIZE);
  }
};

#endif