//
// Created by zjx on 2024/5/26.
//

#ifndef TICKETSYSTEM2024_FILE_BLOCK_HPP
#define TICKETSYSTEM2024_FILE_BLOCK_HPP

#include <fstream>
#include <filesystem>
#include "../util/Exceptions.hpp"
#include "../util/Util.hpp"

using std::string;
using std::fstream;
using std::ifstream;
using std::ofstream;

//encode T into S with fixed length
//use linear cache. for big cache size.
template<typename T, int MAX_SIZE, int MAX_CACHE_COUNT>
class FileBlock {
  typedef T::ENCODE S;
  struct Cache {
    T data;
    bool dirty = false;
  };
  static constexpr int S_SIZE = sizeof(S);
  fstream file;
  string fileName;
  Cache *cacheMap[MAX_SIZE]{nullptr}; //a map from index to cache
  int cacheCount = 0;

  static int getIndex(int loc) {
    return loc / S_SIZE;
  }

  static int getLoc(int index) {
    return index * S_SIZE;
  }

public:
  explicit FileBlock(const string &file_name) : fileName("storage/" + file_name + ".dat") {
    if (!std::filesystem::exists(fileName)) {
      std::filesystem::create_directory("storage");
      file.open(fileName, std::ios::out | std::ios::binary);
      file.close();
    }
    file.open(fileName, std::ios::in | std::ios::out | std::ios::binary);
  }

  void checkCache() {
    if (cacheCount > MAX_CACHE_COUNT) {
      for (int i = 0; i < MAX_SIZE; i++) {
        if (cacheMap[i]) {
          if (cacheMap[i]->dirty) {
            file.seekp(getLoc(i));
            S tmp = cacheMap[i]->data.encode();
            file.write(reinterpret_cast<const char *>(&tmp), S_SIZE);
          }
          delete cacheMap[i];
          cacheMap[i] = nullptr;
        }
      }
      cacheCount = 0;
    }
  }

  ~FileBlock() {
    for (int i = 0; i < MAX_SIZE; i++) {
      if (cacheMap[i]) {
        if (cacheMap[i]->dirty) {
          file.seekp(getLoc(i));
          S tmp = cacheMap[i]->data.encode();
          file.write(reinterpret_cast<const char *>(&tmp), S_SIZE);
        }
        delete cacheMap[i];
      }
    }
    file.close();
  }

  int write(const T &t) {
    S tmp = t.encode();
    int loc = file.seekp(0, std::ios::end).tellp();
    file.write(reinterpret_cast<const char *>(&tmp), S_SIZE);
    int index = getIndex(loc);
    if(index < MAX_SIZE) {
      cacheMap[index] = new Cache();
      cacheMap[index]->data = t;
      cacheCount++;
    } else {
      throw FileSizeExceeded();
    }
    return index;
  }

  T *get(int index, bool dirty) {
    if (cacheMap[index] == nullptr) {
      cacheMap[index] = new Cache();
      file.seekg(getLoc(index));
      S tmp;
      file.read(reinterpret_cast<char *>(&tmp), S_SIZE);
      cacheMap[index]->data = T(tmp);
      cacheCount++;
    }
    if (dirty) {
      cacheMap[index]->dirty = true;
    }
    return &cacheMap[index]->data;
  }
};


#endif
