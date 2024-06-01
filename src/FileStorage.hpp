#ifndef TICKETSYSTEM2024_FILE_STORAGE_HPP
#define TICKETSYSTEM2024_FILE_STORAGE_HPP

#include <fstream>
#include <filesystem>
#include "Exceptions.hpp"

using std::string;
using std::fstream;
using std::ifstream;
using std::ofstream;

template<class T, class INFO, int CACHE_SIZE = 0>
class FileStorage {
  struct Cache {
    T data;
    bool dirty;
  };
  static constexpr int T_SIZE = sizeof(T);
  static constexpr int INFO_SIZE = sizeof(INFO);
  static constexpr int INT_SIZE = sizeof(int);
  fstream file;
  string fileName;
  map<int, Cache> cacheMap;
  int empty;

  int getEmpty() {
    return empty;
  }

  void setEmpty(int x) {
    empty = x;
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
    checkCache();
    file.close();
  }

  void checkCache() {
    if(T_SIZE * cacheMap.size() > CACHE_SIZE) {
      for(const auto &it : cacheMap) {
        if(it.second.dirty) {
          file.seekp(it.first);
          file.write(reinterpret_cast<const char *>(&it.second.data), T_SIZE);
        }
      }
      cacheMap.clear();
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

  int add(const T &t) {
    int loc = getEmpty();
    file.seekg(loc);
    int nxt;
    if (file.peek() == EOF) {
      nxt = loc + T_SIZE;
      file.clear(); //clear EOF flag
    } else {
      file.read(reinterpret_cast<char *>(&nxt), INT_SIZE);
    }
    file.seekp(loc);
    file.write(reinterpret_cast<const char *>(&t), T_SIZE);
    setEmpty(nxt);
    return loc;
  }

  T *get(int loc, bool dirty) {
    auto it = cacheMap.find(loc);
    if (it == cacheMap.end()) {
      it = cacheMap.insert({loc, {}}).first;
      file.seekg(loc);
      file.read(reinterpret_cast<char *>(&it->second.data), T_SIZE);
    }
    if(dirty) {
      it->second.dirty = true;
    }
    return &it->second.data;
  }

  void remove(int loc) {
    auto it = cacheMap.find(loc);
    if(it != cacheMap.end()) {
      cacheMap.erase(it);
    }
    int nxt = getEmpty();
    setEmpty(loc);
    file.seekp(loc);
    file.write(reinterpret_cast<const char *>(&nxt), INT_SIZE);
  }
};

#endif