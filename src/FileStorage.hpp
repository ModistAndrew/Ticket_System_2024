#ifndef BPT_FILE_STORAGE_HPP
#define BPT_FILE_STORAGE_HPP

#include <fstream>
#include <filesystem>
#include <map>
#include <list>

using std::string;
using std::fstream;
using std::ifstream;
using std::ofstream;

template<class T, class INFO, int CACHE_SIZE = 10>
//a simple file storage class without cache
class FileStorage {
  fstream file;
  string fileName;
  static constexpr int T_SIZE = sizeof(T);
  static constexpr int INFO_LEN = sizeof(INFO);
  static constexpr int INT_SIZE = sizeof(int);

  struct container {
    FileStorage *storage;
    T *data;
    bool dirty;
    int index;

    container() : storage(nullptr), data(nullptr), dirty(false), index(-1) {}

    container(FileStorage *s, int i) : storage(s), data(nullptr), dirty(false), index(i) {}

    void markDirty() {
      dirty = true;
    }
  };

  std::map<int, container> cacheMap; //a map from index to cache
  std::list<int> cacheList; //a list to store the order of cache
  std::map<int, std::list<int>::iterator> cacheIter; //a map from index to iterator of cacheList
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

  FileStorage(const INFO& initInfo, const string &file_name) : fileName("storage/" + file_name + ".dat") {
    newFile(initInfo);
    file.open(fileName, std::ios::in | std::ios::out | std::ios::binary);
    file.read(reinterpret_cast<char *>(&info), INFO_LEN);
    file.read(reinterpret_cast<char *>(&empty), INT_SIZE);
  }

  ~FileStorage() {
    file.seekp(0);
    file.write(reinterpret_cast<const char *>(&info), INFO_LEN);
    file.write(reinterpret_cast<const char *>(&empty), INT_SIZE);
    for (auto &c : cacheMap) {
      if(c.second.dirty) {
        file.seekp(c.first);
        file.write(reinterpret_cast<const char *>(c.second.data), T_SIZE);
      }
      free(c.second.data);
    }
    file.close();
  }

  void newFile(const INFO& initInfo) {
    if (std::filesystem::exists(fileName)) {
      return;
    }
    std::filesystem::create_directory("storage");
    file.open(fileName, std::ios::out | std::ios::binary);
    int initEmpty = INFO_LEN + INT_SIZE;
    file.write(reinterpret_cast<const char *>(&initInfo), INFO_LEN);
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
    if (cacheMap.find(index) != cacheMap.end()) {
      cacheList.erase(cacheIter[index]);
      cacheIter[index] = cacheList.insert(cacheList.end(), index);
      container& c = cacheMap[index];
      if(dirty) {
        c.markDirty();
      }
      return c.data;
    }
    if (cacheMap.size() >= CACHE_SIZE) {
      int removeIndex = cacheList.front();
      cacheList.pop_front();
      cacheIter.erase(removeIndex);
      container& c = cacheMap[removeIndex];
      if(c.dirty) {
        file.seekp(removeIndex);
        file.write(reinterpret_cast<const char *>(c.data), T_SIZE);
      }
      free(c.data);
      cacheMap.erase(removeIndex);
    }
    container &c = cacheMap[index];
    c = container(this, index);
    c.data = (T*)malloc(T_SIZE);
    file.seekg(index);
    file.read(reinterpret_cast<char *>(c.data), T_SIZE);
    if(dirty) {
      c.markDirty();
    }
    cacheIter[index] = cacheList.insert(cacheList.end(), index);
    return c.data;
  }

  //make sure the index is valid
  //delete a currently occupied index
  //you should never remove an empty index!
  void remove(int index) {
    if (cacheMap.find(index) != cacheMap.end()) {
      cacheList.erase(cacheIter[index]);
      cacheIter.erase(index);
      container& c = cacheMap[index];
      free(c.data);
      cacheMap.erase(index);
    }
    int nxt = getEmpty();
    setEmpty(index);
    file.seekp(index);
    file.write(reinterpret_cast<const char *>(&nxt), INT_SIZE);
  }
};
#endif //BPT_FILE_STORAGE_HPP