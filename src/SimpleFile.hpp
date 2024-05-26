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
  map<int, std::string> cacheMap;
public:
  SimpleFile(const string &file_name) : fileName("storage/"+file_name+".dat") {
    if (!std::filesystem::exists(fileName)) {
      std::filesystem::create_directory("storage");
      file.open(fileName, std::ios::out | std::ios::binary);
      file.close();
    }
    file.open(fileName, std::ios::in | std::ios::out | std::ios::binary);
  }

  ~SimpleFile() {
    file.close();
  }

  int write(const std::string &str) {
    int ret = file.seekp(0, std::ios::end).tellp();
    file << str << '\n';
    return ret;
  }

  std::string read(int pos) {
    auto it = cacheMap.find(pos);
    if(it != cacheMap.end()) {
      return it->second;
    }
    file.seekg(pos);
    std::string ret;
    std::getline(file, ret);
    cacheMap.insert({pos, ret});
    return ret;
  }
};


#endif //TICKETSYSTEM2024_SIMPLEFILE_HPP
