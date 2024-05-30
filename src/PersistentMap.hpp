//
// Created by zjx on 2024/4/22.
//

#ifndef TICKET_SYSTEM_2024_PERSISTENT_MAP_HPP
#define TICKET_SYSTEM_2024_PERSISTENT_MAP_HPP

#include "Util.hpp"
#include "FileStorage.hpp"

template<typename T, int MAX_SIZE = 20000>
class PersistentMap { //use T::index as key
  struct TreeNode;
  struct LeafNode;

  typedef T::INDEX INDEX;

  static constexpr int SIZE_1 = 3200 / sizeof(T) * 2;
  static constexpr int SIZE_2 = 3200 / sizeof(INDEX) * 2;

  static_assert(SIZE_1 >= 4 && SIZE_1 % 2 == 0, "SIZE_1 must be even and at least 4");
  static_assert(SIZE_2 >= 4 && SIZE_2 % 2 == 0, "SIZE_2 must be even and at least 4");

  class iterator {
    PersistentMap *set;
    LeafNode *leaf;
    int pos;
  public:
    iterator(PersistentMap *set, LeafNode *leaf, int pos) : set(set), leaf(leaf), pos(pos) {}

    iterator &operator++() {
      if (leaf == nullptr) {
        throw;
      }
      pos++;
      if (pos == leaf->size) {
        leaf = leaf->next == -1 ? nullptr : set->getPtr(leaf->next, false).leafNode();
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
  };

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

    bool insert(PersistentMap *set, const T &val, TreeNode *parent, int parentPos) {
      if (isLeaf) {
        return leafNode()->insert(set, val, parent, parentPos);
      } else {
        return treeNode()->insert(set, val, parent, parentPos);
      }
    }

    bool erase(PersistentMap *set, const INDEX &val, TreeNode *parent, int parentPos) {
      if (isLeaf) {
        return leafNode()->erase(set, val, parent, parentPos);
      } else {
        return treeNode()->erase(set, val, parent, parentPos);
      }
    }

    iterator find(PersistentMap *set, const INDEX &val) {
      if (isLeaf) {
        return leafNode()->find(set, val);
      } else {
        return treeNode()->find(set, val);
      }
    }
  };

  struct TreeNode {
    int size = 0; //the number of children
    INDEX index[SIZE_1 - 1];
    int children[SIZE_1];

    iterator find(PersistentMap *set, const INDEX &val) { //find first no less than val
      int p = upper_bound(index, index + size - 1, val) - index;
      return set->getPtr(children[p], false).find(set, val);
    }

    bool insert(PersistentMap *set, const T &val, TreeNode *parent, int pos) { //insert val into this node
      int p = upper_bound(index, index + size - 1, val.index()) - index;
      NodePtr child = set->getPtr(children[p], true);
      if (child.insert(set, val, this, p)) {
        if (size == SIZE_1) {
          postInsert(set, parent, pos);
        }
        return true;
      }
      return false;
    }

    bool erase(PersistentMap *set, const INDEX &val, TreeNode *parent, int pos) { //erase val from this node
      int p = upper_bound(index, index + size - 1, val) - index;
      NodePtr child = set->getPtr(children[p], true);
      if (child.erase(set, val, this, p)) {
        if (size == SIZE_1 / 2 - 1) {
          postErase(set, parent, pos);
        }
        return true;
      }
      return false;
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

    void eraseChild(int pos) { //erase a child after children[pos] and index[pos-1]
      int indexPos = pos;
      int childPos = pos + 1;
      memmove(index + indexPos, index + indexPos + 1, (size - indexPos - 2) * sizeof(INDEX));
      memmove(children + childPos, children + childPos + 1, (size - childPos - 1) * sizeof(int));
      size--;
    }

    void postInsert(PersistentMap *set, TreeNode *parent, int pos) { //when size==SIZE
      TreeNode newNode;
      int half = size / 2;
      newNode.size = half;
      size = half;
      memcpy(newNode.index, index + half, (half - 1) * sizeof(INDEX));
      memcpy(newNode.children, children + half, half * sizeof(int));
      parent->insertChild(set->add(newNode), index[half - 1], pos);
    }

    void postErase(PersistentMap *set, TreeNode *parent, int pos) { //when size==SIZE/2-1
      if (parent->size == 1) { //root
        return;
      }
      if (pos == 0) {
        TreeNode *sibling = set->getPtr(parent->children[pos + 1], true).treeNode();
        if (sibling->size > SIZE_1 / 2) {
          memcpy(index + size - 1, parent->index + pos, sizeof(INDEX));
          memcpy(children + size, sibling->children, sizeof(int));
          memcpy(parent->index + pos, sibling->index, sizeof(INDEX));
          memmove(sibling->index, sibling->index + 1, (sibling->size - 2) * sizeof(INDEX));
          memmove(sibling->children, sibling->children + 1, (sibling->size - 1) * sizeof(int));
          size++;
          sibling->size--;
        } else {
          this->merge(set, sibling, parent, pos);
        }
      } else {
        TreeNode *sibling = set->getPtr(parent->children[pos - 1], true).treeNode();
        if (sibling->size > SIZE_1 / 2) {
          memmove(index + 1, index, (size - 1) * sizeof(INDEX));
          memmove(children + 1, children, size * sizeof(int));
          memcpy(index, parent->index + pos - 1, sizeof(INDEX));
          memcpy(children, sibling->children + sibling->size - 1, sizeof(int));
          memcpy(parent->index + pos - 1, sibling->index + sibling->size - 2, sizeof(INDEX));
          size++;
          sibling->size--;
        } else {
          sibling->merge(set, this, parent, pos - 1);
        }
      }
    }

    void merge(PersistentMap *set, TreeNode *sibling, TreeNode *parent, int pos) { //sibling is parent->children[pos+1]
      memcpy(index + size - 1, parent->index + pos, sizeof(INDEX));
      memcpy(index + size, sibling->index, (sibling->size - 1) * sizeof(INDEX));
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

    iterator find(PersistentMap *set, const INDEX &val) { //find first no less than val
      int p = lower_index_bound(data, data + size, val) - data;
      return p == size ? iterator(set, next == -1 ? nullptr : set->getPtr(next, false).leafNode(), 0) : iterator(set, this, p);
    }

    bool insert(PersistentMap *set, const T &val, TreeNode *parent, int pos) { //insert val into this node
      int p = lower_index_bound(data, data + size, val.index()) - data;
      if (p < size && data[p].index() == val.index()) {
        return false;
      }
      memmove(data + p + 1, data + p, (size - p) * sizeof(T));
      data[p] = val;
      size++;
      if (size == SIZE_2) {
        postInsert(set, parent, pos);
      }
      return true;
    }

    bool erase(PersistentMap *set, const INDEX &val, TreeNode *parent, int pos) { //erase val from this node
      int p = lower_index_bound(data, data + size, val) - data;
      if (p >= size || data[p].index() != val) {
        return false;
      }
      memmove(data + p, data + p + 1, (size - p - 1) * sizeof(T));
      size--;
      if (size == SIZE_2 / 2 - 1) {
        postErase(set, parent, pos);
      }
      return true;
    }

    void postInsert(PersistentMap *set, TreeNode *parent, int pos) { //when size==SIZE
      LeafNode newNode;
      int half = size / 2;
      newNode.size = half;
      size = half;
      memcpy(newNode.data, data + half, half * sizeof(T));
      newNode.next = next;
      next = set->add(newNode);
      parent->insertChild(next, data[half].index(), pos);
    }

    void postErase(PersistentMap *set, TreeNode *parent, int pos) { //when size==SIZE/2-1
      if (parent->size == 1) { //root
        return;
      }
      if (pos == 0) {
        LeafNode *sibling = set->getPtr(parent->children[pos + 1], true).leafNode();
        if (sibling->size > SIZE_2 / 2) {
          memcpy(data + size, sibling->data, sizeof(T)); //copy one here
          memmove(sibling->data, sibling->data + 1, (sibling->size - 1) * sizeof(T)); //delete one from sibling
          memcpy(parent->index + pos, sibling->data, sizeof(INDEX)); //replace parent's index
          size++;
          sibling->size--;
        } else {
          this->merge(set, sibling, parent, pos);
        }
      } else {
        LeafNode *sibling = set->getPtr(parent->children[pos - 1], true).leafNode();
        if (sibling->size > SIZE_2 / 2) {
          memmove(data + 1, data, size * sizeof(T)); //leave one space for copy
          memcpy(data, sibling->data + sibling->size - 1, sizeof(T)); //copy one here
          memcpy(parent->index + pos - 1, data, sizeof(INDEX)); //replace parent's index
          size++;
          sibling->size--;
        } else {
          sibling->merge(set, this, parent, pos - 1);
        }
      }
    }

    void merge(PersistentMap *set, LeafNode *sibling, TreeNode *parent, int pos) { //sibling is parent->children[pos+1]
      memcpy(data + size, sibling->data, sibling->size * sizeof(T));
      size += sibling->size;
      next = sibling->next;
      set->remove(parent->children[pos + 1]);
      parent->eraseChild(pos);
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

  void remove(int index) {
    if (index & 1) {
      treeNodeStorage.remove(index >> 1);
    } else {
      leafNodeStorage.remove(index >> 1);
    }
  }

public:
  int length;
  explicit PersistentMap(std::string file_name) : treeNodeStorage(-1, file_name + "_tree"),
                                                  leafNodeStorage(0, file_name + "_leaf") {
    dummy.size = 1;
    dummy.children[0] = treeNodeStorage.info == -1 ? add(LeafNode()) : treeNodeStorage.info;
    length = leafNodeStorage.info;
  }

  bool empty() {
    return length == 0;
  }

  ~PersistentMap() {
    treeNodeStorage.info = dummy.children[0];
    leafNodeStorage.info = length;
  }

  bool insert(const T &val) {
    bool ret = getRoot().insert(this, val, &dummy, 0);
    if (dummy.size == 2) {
      TreeNode newRoot;
      newRoot.size = 1;
      newRoot.children[0] = add(dummy);
      dummy = newRoot;
    }
    if(ret) {
      length++;
    }
    return ret;
  }

  bool erase(const INDEX &val) {
    bool ret = getRoot().erase(this, val, &dummy, 0);
    NodePtr root = getRoot();
    if (!root.isLeaf) {
      TreeNode rootNode = *root.treeNode();
      if (rootNode.size == 1) {
        remove(dummy.children[0]);
        dummy = rootNode;
      }
    }
    if(ret) {
      length--;
    }
    return ret;
  }

  pair<iterator, bool> get(const INDEX &val) { //return the iterator first no less than val and whether it equals val
    iterator it = getRoot().find(this, val);
    return {it, !it.end() && it->index() == val};
  }
};

#endif //TICKET_SYSTEM_2024_PERSISTENT_MAP_HPP