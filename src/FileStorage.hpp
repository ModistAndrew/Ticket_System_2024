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
  INFO info;
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
  string fileName;

  FileStorage() = default;

  explicit FileStorage(const string &file_name) : fileName("storage/" + file_name + ".dat") {
  }

  void init() {
    newFile();
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

  void newFile() {
    if (std::filesystem::exists(fileName)) {
      return;
    }
    std::filesystem::create_directory("storage");
    file.open(fileName, std::ios::out | std::ios::binary);
    INFO initInfo;
    int initEmpty = INFO_LEN + INT_SIZE;
    file.write(reinterpret_cast<const char *>(&initInfo), INFO_LEN);
    file.write(reinterpret_cast<const char *>(&initEmpty), INT_SIZE);
    file.close();
  }

  INFO& getInfo() {
    return info;
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
  //the return value is a pointer to the object in cache, which will be invalid after the cache is destroyed. so don't store it.
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
      int idx = cacheList.front();
      cacheList.pop_front();
      cacheIter.erase(idx);
      container& c = cacheMap[index];
      if(c.dirty) {
        file.seekp(idx);
        file.write(reinterpret_cast<const char *>(c.data), T_SIZE);
      }
      free(c.data);
      cacheMap.erase(idx);
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

#ifndef BPT_MEMORYRIVER_HPP
#define BPT_MEMORYRIVER_HPP

#include <fstream>

using std::string;
using std::fstream;
using std::ifstream;
using std::ofstream;

template<class T, int info_len = 2>
class MemoryRiver {
  FileStorage<T, int[info_len]> file;
public:
  MemoryRiver() = default;

  MemoryRiver(const string& file_name) : file(file_name) {
  }

  void initialise(string FN = "") {
    if (FN != "") {
      file.fileName = FN;
    }
    file.init();
    for(int i = 0; i < info_len; i++) {
      file.getInfo()[i] = 0;
    }
  }

  //读出第n个int的值赋给tmp，1_base
  void get_info(int &tmp, int n) {
    if (n > info_len) return;
    tmp = file.getInfo()[n - 1];
  }

  //将tmp写入第n个int的位置，1_base
  void write_info(int tmp, int n) {
    if (n > info_len) return;
    file.getInfo()[n - 1] = tmp;
  }

  int write(T &t) {
    return file.add(t);
  }

  void update(T &t, const int index) {
    *file.get(index, true) = t;
  }

  //读出位置索引index对应的T对象的值并赋值给t，保证调用的index都是由write函数产生
  void read(T &t, const int index) {
    t = *file.get(index, false);
  }

  //删除位置索引index对应的对象(不涉及空间回收时，可忽略此函数)，保证调用的index都是由write函数产生
  void Delete(int index) {
    file.remove(index);
  }
};


#endif //BPT_MEMORYRIVER_HPP