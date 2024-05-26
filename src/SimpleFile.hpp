//
// Created by zjx on 2024/5/26.
//

#ifndef TICKETSYSTEM2024_SIMPLEFILE_HPP
#define TICKETSYSTEM2024_SIMPLEFILE_HPP

#include <fstream>
#include <filesystem>
#include "Exceptions.hpp"
#include "map.hpp"

using std::string;
using std::fstream;
using std::ifstream;
using std::ofstream;

class SimpleFile { //you can write a string at end of file and read a string randomly
  fstream file;
  string fileName;
  struct Cache {
    std::string data;
    bool dirty = false;
  };
  map<int, Cache *> cacheMap;
public:
  SimpleFile(const string &file_name) : fileName("storage/" + file_name + ".dat") {
    if (!std::filesystem::exists(fileName)) {
      std::filesystem::create_directory("storage");
      file.open(fileName, std::ios::out | std::ios::binary);
      file.close();
    }
    file.open(fileName, std::ios::in | std::ios::out | std::ios::binary);
  }

  ~SimpleFile() {
    file.close();
    for (auto i: cacheMap) {
      if (i.second->dirty) {
        file.seekp(i.first);
        file << i.second->data;
      }
      delete i.second;
    }
  }

  int write(const std::string &str) {
    int ret = file.seekp(0, std::ios::end).tellp();
    file << str << '\n';
    return ret;
  }

  std::string *get(int pos, bool dirty) {
    auto it = cacheMap.find(pos);
    if (it == cacheMap.end()) {
      it = cacheMap.insert({pos, new Cache()}).first;
      file.seekg(pos);
      std::getline(file, it->second->data);
    }
    if (dirty) {
      it->second->dirty = true;
    }
    return &it->second->data;
  }
};


#endif //TICKETSYSTEM2024_SIMPLEFILE_HPP
