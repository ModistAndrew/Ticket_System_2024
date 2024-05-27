//
// Created by zjx on 2024/4/22.
//

#ifndef TICKET_SYSTEM_2024_PERSISTENT_MULTI_MAP_HPP
#define TICKET_SYSTEM_2024_PERSISTENT_MULTI_MAP_HPP

#include "Util.hpp"
#include "FileStorage.hpp"

template<typename T, int MAX_SIZE = 20000>
class PersistentMultiMap { //use INDEX as key and T as value. Data can only be inserted in order. new data with the same index will be inserted at the beginning of the linked list
  struct TreeNode;
  struct LeafNode;

  typedef decltype(T::index) INDEX;

  static constexpr int SIZE_1 = 3200 / sizeof(T) * 2;
  static constexpr int SIZE_2 = 3200 / sizeof(INDEX) * 2;

  static_assert(SIZE_1 >= 4 && SIZE_1 % 2 == 0, "SIZE_1 must be even and at least 4");
  static_assert(SIZE_2 >= 4 && SIZE_2 % 2 == 0, "SIZE_2 must be even and at least 4");

  class iterator {
    PersistentMultiMap *set;
    int leafPos;
    int pos;
    LeafNode *leaf;

  public:
    iterator(PersistentMultiMap *set, int leafPos, int pos) : set(set), leafPos(leafPos), pos(pos),
                                                              leaf(set->getPtr(leafPos, false).leafNode()) {}

    iterator &operator++() {
      if (leaf == nullptr) {
        throw;
      }
      pos++;
      if (pos == leaf->size) {
        leafPos = leaf->next;
        leaf = set->getPtr(leafPos, false).leafNode();
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

    T *operator->() {
      if (leaf == nullptr) {
        throw;
      }
      return &leaf->data[pos];
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

    void markDirty() { //set current node as dirty
      set->getPtr(leafPos, true);
    }
  };

  iterator end() {
    return iterator(this, -1, 0);
  }

  class NodePtr {
    void *ptr;
  public:
    bool isLeaf;

    explicit NodePtr(TreeNode *ptr) : ptr(ptr), isLeaf(false) {}

    explicit NodePtr(LeafNode *ptr) : ptr(ptr), isLeaf(true) {}

    NodePtr() : ptr(nullptr), isLeaf(false) {}

    TreeNode *treeNode() {
      return static_cast<TreeNode *>(ptr);
    }

    LeafNode *leafNode() {
      return static_cast<LeafNode *>(ptr);
    }

    void insert(PersistentMultiMap *set, const T &val, TreeNode *parent, int parentPos) {
      if (isLeaf) {
        leafNode()->insert(set, val, parent, parentPos);
      } else {
        treeNode()->insert(set, val, parent, parentPos);
      }
    }

    iterator find(PersistentMultiMap *set, const INDEX &val, int loc) { //find first of the linked list under val. return end() if not found
      if (isLeaf) {
        return leafNode()->find(set, val, loc);
      } else {
        return treeNode()->find(set, val, loc);
      }
    }
  };

  struct TreeNode {
    int size = 0; //the number of children
    INDEX index[SIZE_1 - 1];
    int children[SIZE_1];

    iterator find(PersistentMultiMap *set, const INDEX &val, int loc) { //find first no less than val
      //notice that we have (a, b] rather than [a, b) here in order to get the first element of the linked list with lower_bound
      int p = lower_bound(index, index + size - 1, val) - index;
      return set->getPtr(children[p], false).find(set, val, children[p]);
    }

    void insert(PersistentMultiMap *set, const T &val, TreeNode *parent, int pos) { //insert val into this node
      //notice that we have (a, b] rather than [a, b) here in order to get the first element of the linked list with lower_bound
      int p = lower_bound(index, index + size - 1, val.index) - index;
      NodePtr child = set->getPtr(children[p], true);
      child.insert(set, val, this, p);
      if (size == SIZE_1) {
        postInsert(set, parent, pos);
      }
    }

    void insertChild(int newChild, const INDEX &newIndex,
                     int pos) { //insert newChild after children[pos] and newIndex after index[pos-1]
      int indexPos = pos;
      int childPos = pos + 1;
      memmove(index + indexPos + 1, index + indexPos, (size - indexPos - 1) * sizeof(INDEX));
      memmove(children + childPos + 1, children + childPos, (size - childPos) * sizeof(int));
      index[indexPos] = newIndex;
      children[childPos] = newChild;
      size++;
    }

    void postInsert(PersistentMultiMap *set, TreeNode *parent, int pos) { //when size==SIZE
      TreeNode newNode;
      int half = size / 2;
      newNode.size = half;
      size = half;
      memcpy(newNode.index, index + half, (half - 1) * sizeof(INDEX));
      memcpy(newNode.children, children + half, half * sizeof(int));
      parent->insertChild(set->add(newNode), index[half - 1], pos);
    }
  };

  struct LeafNode {
    int size = 0;
    T data[SIZE_2];
    int next = -1; //linked list

    iterator find(PersistentMultiMap *set, const INDEX &val, int loc) { //find first no less than val
      int p = lower_index_bound(data, data + size, val) - data;
      if (p < size && data[p].index == val) {
        return iterator(set, loc, p);
      }
      return set->end();
    }

    void insert(PersistentMultiMap *set, const T &val, TreeNode *parent, int pos) { //insert val into this node
      int p = lower_index_bound(data, data + size, val.index) - data;
      memmove(data + p + 1, data + p, (size - p) * sizeof(T));
      data[p] = val;
      size++;
      if (size == SIZE_2) {
        postInsert(set, parent, pos);
      }
    }

    void postInsert(PersistentMultiMap *set, TreeNode *parent, int pos) { //when size==SIZE
      LeafNode newNode;
      int half = size / 2;
      newNode.size = half;
      size = half;
      memcpy(newNode.data, data + half, half * sizeof(T));
      newNode.next = next;
      next = set->add(newNode);
      parent->insertChild(next, data[half - 1].index,
                          pos); //notice that we have (a, b] rather than [a, b) here in order to get the first element of the linked list with lower_bound
    }
  };

  TreeNode dummy; //there is a fake tree node which always points to the root
  FileStorage<TreeNode, int, MAX_SIZE> treeNodeStorage; //int is the index of the root
  FileStorage<LeafNode, int, MAX_SIZE> leafNodeStorage; //int is the size

  NodePtr getPtr(int index, bool dirty) {
    if (index == -1) {
      return NodePtr();
    }
    if (index & 1) {
      return NodePtr(treeNodeStorage.get(index >> 1, dirty));
    }
    return NodePtr(leafNodeStorage.get(index >> 1, dirty));
  }

  NodePtr getRoot() {
    return getPtr(dummy.children[0], true);
  }

  int add(const TreeNode &node) {
    return treeNodeStorage.add(node) << 1 | 1;
  }

  int add(const LeafNode &node) {
    return leafNodeStorage.add(node) << 1;
  }

public:
  int length;

  explicit PersistentMultiMap(std::string file_name) :
    treeNodeStorage(-1, file_name + "_tree"), leafNodeStorage(0, file_name + "_leaf") {
    dummy.size = 1;
    dummy.children[0] = treeNodeStorage.info == -1 ? add(LeafNode()) : treeNodeStorage.info;
    length = leafNodeStorage.info;
  }

  bool empty() {
    return length == 0;
  }

  ~PersistentMultiMap() {
    treeNodeStorage.info = dummy.children[0];
  }

  void insert(const T &val) {
    getRoot().insert(this, val, &dummy, 0);
    if (dummy.size == 2) {
      TreeNode newRoot;
      newRoot.size = 1;
      newRoot.children[0] = add(dummy);
      dummy = newRoot;
    }
    length++;
  }

  iterator find(const INDEX &val) {
    return getRoot().find(this, val, dummy.children[0]);
  }
};

#endif //TICKET_SYSTEM_2024_PERSISTENT_MULTI_MAP_HPP