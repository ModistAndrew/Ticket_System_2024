//
// Created by zjx on 2024/4/22.
//

#ifndef TICKET_SYSTEM_2024_PERSISTENT_SET_HPP
#define TICKET_SYSTEM_2024_PERSISTENT_SET_HPP

#include "Util.hpp"
#include "FileStorage.hpp"

template<typename T, int SIZE_1, int SIZE_2, int CACHE_SIZE>
class PersistentSet {
  static_assert(SIZE_1 >= 4 && SIZE_1 % 2 == 0, "SIZE_1 must be even and at least 4");
  static_assert(SIZE_2 >= 4 && SIZE_2 % 2 == 0, "SIZE_2 must be even and at least 4");

  struct TreeNode;
  struct LeafNode;

  class iterator {
    PersistentSet *set;
    LeafNode *leaf;
    int pos;
  public:
    iterator(PersistentSet *set, LeafNode *leaf, int pos) : set(set), leaf(leaf), pos(pos) {}

    iterator &operator++() {
      if (leaf == nullptr) {
        throw;
      }
      pos++;
      if (pos == leaf->size) {
        leaf = leaf->next == -1 ? nullptr : &set->getPtr(leaf->next).leafNodeCache()->data;
        pos = 0;
      }
      return *this;
    }

    iterator operator++(int) {
      iterator ret = *this;
      ++(*this);
      return ret;
    }

    T &operator*() {
      if (leaf == nullptr) {
        throw;
      }
      return leaf->data[pos];
    }

    bool operator==(const iterator &rhs) const {
      return leaf == rhs.leaf && pos == rhs.pos && set == rhs.set;
    }

    bool operator!=(const iterator &rhs) const {
      return leaf != rhs.leaf || pos != rhs.pos || set != rhs.set;
    }

    bool end() {
      return leaf == nullptr;
    }
  };

  //return flag:1 for self dirty, 2 for parent dirty, 4 for success
  static int parentFlag(int child) {
    return (child & 4) + ((child & 2) >> 1);
  }

  class NodePtr {
    void *ptr;
  public:
    bool isLeaf;

    explicit NodePtr(FileStorage<TreeNode, int, CACHE_SIZE>::Cache *ptr) : ptr(ptr), isLeaf(false) {}

    explicit NodePtr(FileStorage<LeafNode, int, CACHE_SIZE>::Cache *ptr) : ptr(ptr), isLeaf(true) {}

    FileStorage<TreeNode, int, CACHE_SIZE>::Cache *treeNodeCache() {
      return static_cast<FileStorage<TreeNode, int, CACHE_SIZE>::Cache *>(ptr);
    }

    FileStorage<LeafNode, int, CACHE_SIZE>::Cache *leafNodeCache() {
      return static_cast<FileStorage<LeafNode, int, CACHE_SIZE>::Cache *>(ptr);
    }

    void markDirty() {
      if (isLeaf) {
        leafNodeCache()->dirty = true;
      } else {
        treeNodeCache()->dirty = true;
      }
    }

    int insert(PersistentSet *set, const T &val, TreeNode *parent, int parentPos) {
      if (isLeaf) {
        auto cache = leafNodeCache();
        int ret = cache->data.insert(set, val, parent, parentPos);
        if (ret & 1) {
          cache->dirty = true;
        }
        return ret;
      } else {
        auto cache = treeNodeCache();
        int ret = cache->data.insert(set, val, parent, parentPos);
        if (ret & 1) {
          cache->dirty = true;
        }
        return ret;
      }
    }

    int erase(PersistentSet *set, const T &val, TreeNode *parent, int parentPos) {
      if (isLeaf) {
        auto cache = leafNodeCache();
        int ret = cache->data.erase(set, val, parent, parentPos);
        if (ret & 1) {
          cache->dirty = true;
        }
        return ret;
      } else {
        auto cache = treeNodeCache();
        int ret = cache->data.erase(set, val, parent, parentPos);
        if (ret & 1) {
          cache->dirty = true;
        }
        return ret;
      }
    }

    iterator find(PersistentSet *set, const T &val) {
      if (isLeaf) {
        return leafNodeCache()->data.find(set, val);
      } else {
        return treeNodeCache()->data.find(set, val);
      }
    }
  };

  struct TreeNode {
    int size = 0; //the number of children
    T index[SIZE_1 - 1];
    int children[SIZE_1];

    iterator find(PersistentSet *set, const T &val) { //find first no less than val
      int p = upper_bound(index, index + size - 1, val) - index;
      return set->getPtr(children[p]).find(set, val);
    }

    int insert(PersistentSet *set, const T &val, TreeNode *parent, int pos) { //insert val into this node
      int p = upper_bound(index, index + size - 1, val) - index;
      NodePtr child = set->getPtr(children[p]);
      int flag = parentFlag(child.insert(set, val, this, p));
      if (flag & 1) {
        if (size == SIZE_1) {
          flag |= 2;
          postInsert(set, parent, pos);
        }
      }
      return flag;
    }

    int erase(PersistentSet *set, const T &val, TreeNode *parent, int pos) { //erase val from this node
      int p = upper_bound(index, index + size - 1, val) - index;
      NodePtr child = set->getPtr(children[p]);
      int flag = parentFlag(child.erase(set, val, this, p));
      if (flag & 1) {
        if (size == SIZE_1 / 2 - 1) {
          flag |= 2;
          postErase(set, parent, pos);
        }
      }
      return flag;
    }

    void insertChild(int newChild, const T &newIndex,
                     int pos) { //insert newChild after children[pos] ans newIndex after index[pos-1]
      int indexPos = pos;
      int childPos = pos + 1;
      memmove(index + indexPos + 1, index + indexPos, (size - indexPos - 1) * sizeof(T));
      memmove(children + childPos + 1, children + childPos, (size - childPos) * sizeof(int));
      index[indexPos] = newIndex;
      children[childPos] = newChild;
      size++;
    }

    void eraseChild(int pos) { //erase a child after children[pos] and index[pos-1]
      int indexPos = pos;
      int childPos = pos + 1;
      memmove(index + indexPos, index + indexPos + 1, (size - indexPos - 2) * sizeof(T));
      memmove(children + childPos, children + childPos + 1, (size - childPos - 1) * sizeof(int));
      size--;
    }

    void postInsert(PersistentSet *set, TreeNode *parent, int pos) { //when size==SIZE
      TreeNode newNode;
      int half = size / 2;
      newNode.size = half;
      size = half;
      memcpy(newNode.index, index + half, (half - 1) * sizeof(T));
      memcpy(newNode.children, children + half, half * sizeof(int));
      parent->insertChild(set->add(newNode), index[half - 1], pos);
    }

    void postErase(PersistentSet *set, TreeNode *parent, int pos) { //when size==SIZE/2-1
      if (parent->size == 1) { //root
        return;
      }
      if (pos == 0) {
        auto cache = set->getPtr(parent->children[pos + 1]).treeNodeCache();
        cache->dirty = true;
        TreeNode *sibling = &cache->data;
        if (sibling->size > SIZE_1 / 2) {
          memcpy(index + size - 1, parent->index + pos, sizeof(T));
          memcpy(children + size, sibling->children, sizeof(int));
          memcpy(parent->index + pos, sibling->index, sizeof(T));
          memmove(sibling->index, sibling->index + 1, (sibling->size - 2) * sizeof(T));
          memmove(sibling->children, sibling->children + 1, (sibling->size - 1) * sizeof(int));
          size++;
          sibling->size--;
        } else {
          this->merge(set, sibling, parent, pos);
        }
      } else {
        auto cache = set->getPtr(parent->children[pos - 1]).treeNodeCache();
        cache->dirty = true;
        TreeNode *sibling = &cache->data;
        if (sibling->size > SIZE_1 / 2) {
          memmove(index + 1, index, (size - 1) * sizeof(T));
          memmove(children + 1, children, size * sizeof(int));
          memcpy(index, parent->index + pos - 1, sizeof(T));
          memcpy(children, sibling->children + sibling->size - 1, sizeof(int));
          memcpy(parent->index + pos - 1, sibling->index + sibling->size - 2, sizeof(T));
          size++;
          sibling->size--;
        } else {
          sibling->merge(set, this, parent, pos - 1);
        }
      }
    }

    void merge(PersistentSet *set, TreeNode *sibling, TreeNode *parent, int pos) { //sibling is parent->children[pos+1]
      memcpy(index + size - 1, parent->index + pos, sizeof(T));
      memcpy(index + size, sibling->index, (sibling->size - 1) * sizeof(T));
      memcpy(children + size, sibling->children, sibling->size * sizeof(int));
      size += sibling->size;
      set->remove(parent->children[pos + 1]);
      parent->eraseChild(pos);
    }
  };

  struct LeafNode {
    int size = 0;
    T data[SIZE_2];
    int next = -1; //linked list

    iterator find(PersistentSet *set, const T &val) { //find first no less than val
      int p = lower_bound(data, data + size, val) - data;
      return p == size ? iterator(set, next == -1 ? nullptr : &set->getPtr(next).leafNodeCache()->data, 0) : iterator(
        set, this, p);
    }

    int insert(PersistentSet *set, const T &val, TreeNode *parent, int pos) { //insert val into this node
      int p = lower_bound(data, data + size, val) - data;
      if (p < size && data[p] == val) {
        return 0;
      }
      memmove(data + p + 1, data + p, (size - p) * sizeof(T));
      data[p] = val;
      size++;
      if (size == SIZE_2) {
        postInsert(set, parent, pos);
        return 7;
      }
      return 5;
    }

    int erase(PersistentSet *set, const T &val, TreeNode *parent, int pos) { //erase val from this node
      int p = lower_bound(data, data + size, val) - data;
      if (p >= size || data[p] != val) {
        return 0;
      }
      memmove(data + p, data + p + 1, (size - p - 1) * sizeof(T));
      size--;
      if (size == SIZE_2 / 2 - 1) {
        postErase(set, parent, pos);
        return 7;
      }
      return 5;
    }

    void postInsert(PersistentSet *set, TreeNode *parent, int pos) { //when size==SIZE
      LeafNode newNode;
      int half = size / 2;
      newNode.size = half;
      size = half;
      memcpy(newNode.data, data + half, half * sizeof(T));
      newNode.next = next;
      next = set->add(newNode);
      parent->insertChild(next, data[half], pos);
    }

    void postErase(PersistentSet *set, TreeNode *parent, int pos) { //when size==SIZE/2-1
      if (parent->size == 1) { //root
        return;
      }
      if (pos == 0) {
        auto cache = set->getPtr(parent->children[pos + 1]).leafNodeCache();
        cache->dirty = true;
        LeafNode *sibling = &cache->data;
        if (sibling->size > SIZE_2 / 2) {
          memcpy(data + size, sibling->data, sizeof(T)); //copy one here
          memmove(sibling->data, sibling->data + 1, (sibling->size - 1) * sizeof(T)); //delete one from sibling
          memcpy(parent->index + pos, sibling->data, sizeof(T)); //replace parent's index
          size++;
          sibling->size--;
        } else {
          this->merge(set, sibling, parent, pos);
        }
      } else {
        auto cache = set->getPtr(parent->children[pos - 1]).leafNodeCache();
        cache->dirty = true;
        LeafNode *sibling = &cache->data;
        if (sibling->size > SIZE_2 / 2) {
          memmove(data + 1, data, size * sizeof(T)); //leave one space for copy
          memcpy(data, sibling->data + sibling->size - 1, sizeof(T)); //copy one here
          memcpy(parent->index + pos - 1, data, sizeof(T)); //replace parent's index
          size++;
          sibling->size--;
        } else {
          sibling->merge(set, this, parent, pos - 1);
        }
      }
    }

    void merge(PersistentSet *set, LeafNode *sibling, TreeNode *parent, int pos) { //sibling is parent->children[pos+1]
      memcpy(data + size, sibling->data, sibling->size * sizeof(T));
      size += sibling->size;
      next = sibling->next;
      set->remove(parent->children[pos + 1]);
      parent->eraseChild(pos);
    }
  };

  TreeNode dummy; //there is a fake tree node which always points to the root
  FileStorage<TreeNode, int, CACHE_SIZE> treeNodeStorage;
  FileStorage<LeafNode, int, CACHE_SIZE> leafNodeStorage;

  NodePtr getPtr(int index) {
    if (index & 1) {
      return NodePtr(treeNodeStorage.get(index >> 1));
    }
    return NodePtr(leafNodeStorage.get(index >> 1));
  }

  NodePtr getRoot() {
    return getPtr(dummy.children[0]);
  }

  int add(const TreeNode &node) {
    return treeNodeStorage.add(node) << 1 | 1;
  }

  int add(const LeafNode &node) {
    return leafNodeStorage.add(node) << 1;
  }

  void remove(int index) {
    if (index & 1) {
      treeNodeStorage.remove(index >> 1);
    } else {
      leafNodeStorage.remove(index >> 1);
    }
  }

public:
  explicit PersistentSet(std::string file_name) : treeNodeStorage(-1, file_name + "_tree"),
                                                  leafNodeStorage(-1, file_name + "_leaf") {
    dummy.size = 1;
    dummy.children[0] = treeNodeStorage.info == -1 ? add(LeafNode()) : treeNodeStorage.info;
  }

  ~PersistentSet() {
    treeNodeStorage.info = dummy.children[0];
  }

  void checkCache() {
    treeNodeStorage.checkCache();
    leafNodeStorage.checkCache();
  }

  bool insert(const T &val) {
    bool ret = getRoot().insert(this, val, &dummy, 0);
    if (dummy.size == 2) {
      TreeNode newRoot;
      newRoot.size = 1;
      newRoot.children[0] = add(dummy);
      dummy = newRoot;
    }
    return ret;
  }

  bool erase(const T &val) {
    bool ret = getRoot().erase(this, val, &dummy, 0);
    NodePtr root = getRoot();
    if (!root.isLeaf) {
      TreeNode rootNode = root.treeNodeCache()->data;
      if (rootNode.size == 1) {
        remove(dummy.children[0]);
        dummy = rootNode;
      }
    }
    return ret;
  }

  iterator lowerBound(const T &val) {
    return getRoot().find(this, val);
  }

  iterator upperBound(const T &val) {
    iterator it = lowerBound(val);
    if (!it.end() && *it == val) {
      ++it;
    }
    return it;
  }
};

#endif //TICKET_SYSTEM_2024_PERSISTENT_SET_HPP