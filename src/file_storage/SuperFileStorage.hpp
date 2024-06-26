#ifndef TICKETSYSTEM2024_SUPER_FILE_STORAGE_HPP
#define TICKETSYSTEM2024_SUPER_FILE_STORAGE_HPP

#include <fstream>
#include <filesystem>
#include "../util/Exceptions.hpp"

using std::string;
using std::fstream;
using std::ifstream;
using std::ofstream;

//file storage which saves all data in cache
//use linear cache. for big cache size. (in fact, it's not a cache)
template<class T, class INFO, int MAX_SIZE>
class SuperFileStorage {
  struct Cache {
    T data;
    bool dirty = false;
  };
  static constexpr int T_SIZE = sizeof(T);
  static constexpr int INFO_SIZE = sizeof(INFO);
  static constexpr int INT_SIZE = sizeof(int);
  fstream file;
  string fileName;
  Cache* cacheMap[MAX_SIZE]{nullptr}; //a map from index to cache
  int empty;

  int getEmpty() {
    return empty;
  }

  void setEmpty(int x) {
    empty = x;
  }

  static int getIndex(int loc) {
    return (loc - INFO_SIZE - INT_SIZE) / T_SIZE;
  }

  static int getLoc(int index) {
    return index * T_SIZE + INFO_SIZE + INT_SIZE;
  }
  //store pointer to first empty just after info len.
  //empty except end has pointer to next empty; check EOF to determine whether at end. (so don't store anything after this)
public:
  INFO info;

  SuperFileStorage(const INFO &initInfo, const string &file_name) : fileName("storage/" + file_name + ".dat") {
    newFile(initInfo);
    file.open(fileName, std::ios::in | std::ios::out | std::ios::binary);
    file.read(reinterpret_cast<char *>(&info), INFO_SIZE);
    file.read(reinterpret_cast<char *>(&empty), INT_SIZE);
  }

  ~SuperFileStorage() {
    file.seekp(0);
    file.write(reinterpret_cast<const char *>(&info), INFO_SIZE);
    file.write(reinterpret_cast<const char *>(&empty), INT_SIZE);
    for(int i= 0; i < MAX_SIZE; i++) {
      if(cacheMap[i]) {
        if(cacheMap[i]->dirty) {
          file.seekp(getLoc(i));
          file.write(reinterpret_cast<const char *>(&cacheMap[i]->data), T_SIZE);
        }
        delete cacheMap[i];
      }
    }
    file.close();
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
    int index = getIndex(loc);
    if(index < MAX_SIZE) {
      cacheMap[index] = new Cache();
      cacheMap[index]->data = t;
    } else {
      throw FileSizeExceeded();
    }
    return index;
  }

  T *get(int index, bool dirty) {
    if(!cacheMap[index]) {
      cacheMap[index] = new Cache();
      file.seekg(getLoc(index));
      file.read(reinterpret_cast<char *>(&cacheMap[index]->data), T_SIZE);
    }
    if(dirty) {
      cacheMap[index]->dirty = true;
    }
    return &cacheMap[index]->data;
  }

  void remove(int index) {
    if(cacheMap[index]) {
      delete cacheMap[index];
      cacheMap[index] = nullptr;
    }
    int nxt = getEmpty();
    int loc = getLoc(index);
    setEmpty(loc);
    file.seekp(loc);
    file.write(reinterpret_cast<const char *>(&nxt), INT_SIZE);
  }
};

#endif