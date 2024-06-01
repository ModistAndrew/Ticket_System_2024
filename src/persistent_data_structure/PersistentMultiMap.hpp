//
// Created by zjx on 2024/4/22.
//

#ifndef TICKETSYSTEM2024_PERSISTENT_MULTI_MAP_HPP
#define TICKETSYSTEM2024_PERSISTENT_MULTI_MAP_HPP

#include "../util/Util.hpp"
#include "../file_storage/FileStorage.hpp"
#include "../file_storage/SuperFileStorage.hpp"

template<typename T0, int MAX_TREE_SIZE = 1000>
class PersistentMultiMap {
  //use T0+int as key and value. new elements are always inserted at end or first
  //if you want other order, use persistent set instead
  struct TreeNode;
  struct LeafNode;

  struct INDEX {
    T0::INDEX index;
    int tick;
    auto operator<=>(const INDEX &rhs) const = default;
  };

  struct T {
    T0 val;
    int tick;

    INDEX index() const {
      return {val.index(), tick};
    }
  };

  static constexpr int SIZE_1 = 1000 / sizeof(T) * 2;
  static constexpr int SIZE_2 = 1000 / sizeof(INDEX) * 2;

  static_assert(SIZE_1 >= 4 && SIZE_1 % 2 == 0, "SIZE_1 must be even and at least 4");
  static_assert(SIZE_2 >= 4 && SIZE_2 % 2 == 0, "SIZE_2 must be even and at least 4");

  class iterator {
    PersistentMultiMap *set;
    int leafPos;
    int pos;
    LeafNode *leaf;

  public:
    iterator() = default;

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

    bool insert(PersistentMultiMap *set, const T &val, TreeNode *parent, int parentPos) {
      if (isLeaf) {
        return leafNode()->insert(set, val, parent, parentPos);
      } else {
        return treeNode()->insert(set, val, parent, parentPos);
      }
    }

    bool erase(PersistentMultiMap *set, const INDEX &val, TreeNode *parent, int parentPos) {
      if (isLeaf) {
        return leafNode()->erase(set, val, parent, parentPos);
      } else {
        return treeNode()->erase(set, val, parent, parentPos);
      }
    }

    iterator find(PersistentMultiMap *set, const INDEX &val, int loc) {
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
      int p = upper_bound(index, index + size - 1, val) - index;
      return set->getPtr(children[p], false).find(set, val, children[p]);
    }

    bool insert(PersistentMultiMap *set, const T &val, TreeNode *parent, int pos) { //insert val into this node
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

    bool erase(PersistentMultiMap *set, const INDEX &val, TreeNode *parent, int pos) { //erase val from this node
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

    void postInsert(PersistentMultiMap *set, TreeNode *parent, int pos) { //when size==SIZE
      TreeNode newNode;
      int half = size / 2;
      newNode.size = half;
      size = half;
      memcpy(newNode.index, index + half, (half - 1) * sizeof(INDEX));
      memcpy(newNode.children, children + half, half * sizeof(int));
      parent->insertChild(set->add(newNode), index[half - 1], pos);
    }

    void postErase(PersistentMultiMap *set, TreeNode *parent, int pos) { //when size==SIZE/2-1
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

    void merge(PersistentMultiMap *set, TreeNode *sibling, TreeNode *parent, int pos) { //sibling is parent->children[pos+1]
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

    iterator find(PersistentMultiMap *set, const INDEX &val, int loc) { //find first no less than val
      int p = lower_index_bound(data, data + size, val) - data;
      return p == size ? iterator(set, next, 0) : iterator(set, loc, p);
    }

    bool insert(PersistentMultiMap *set, const T &val, TreeNode *parent, int pos) { //insert val into this node
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

    bool erase(PersistentMultiMap *set, const INDEX &val, TreeNode *parent, int pos) { //erase val from this node
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

    void postInsert(PersistentMultiMap *set, TreeNode *parent, int pos) { //when size==SIZE
      LeafNode newNode;
      int half = size / 2;
      newNode.size = half;
      size = half;
      memcpy(newNode.data, data + half, half * sizeof(T));
      newNode.next = next;
      next = set->add(newNode);
      parent->insertChild(next, data[half].index(), pos);
    }

    void postErase(PersistentMultiMap *set, TreeNode *parent, int pos) { //when size==SIZE/2-1
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

    void merge(PersistentMultiMap *set, LeafNode *sibling, TreeNode *parent, int pos) { //sibling is parent->children[pos+1]
      memcpy(data + size, sibling->data, sibling->size * sizeof(T));
      size += sibling->size;
      next = sibling->next;
      set->remove(parent->children[pos + 1]);
      parent->eraseChild(pos);
    }
  };

  TreeNode dummy; //there is a fake tree node which always points to the root
  SuperFileStorage<TreeNode, int, MAX_TREE_SIZE> treeNodeStorage; //int is the index of the root
  FileStorage<LeafNode, int, 0> leafNodeStorage; //int is total

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
  int total;
  explicit PersistentMultiMap(std::string file_name) : treeNodeStorage(-1, file_name + "_tree"),
                                                       leafNodeStorage(0, file_name + "_leaf") {
    dummy.size = 1;
    dummy.children[0] = treeNodeStorage.info == -1 ? add(LeafNode()) : treeNodeStorage.info;
    total = leafNodeStorage.info;
  }

  void checkCache() {
    leafNodeStorage.checkCache();
  }

  ~PersistentMultiMap() {
    treeNodeStorage.info = dummy.children[0];
    leafNodeStorage.info = total;
  }

  int pushBack(const T0 &val) {
    int ret = total;
    total++;
    if(total <= 0) {
      throw;
    }
    getRoot().insert(this, {val, ret}, &dummy, 0);
    if (dummy.size == 2) {
      TreeNode newRoot;
      newRoot.size = 1;
      newRoot.children[0] = add(dummy);
      dummy = newRoot;
    }
    return ret;
  }

  int pushFront(const T0 &val) {
    int ret = -total;
    total++;
    if(total <= 0) {
      throw;
    }
    getRoot().insert(this, {val, ret}, &dummy, 0);
    if (dummy.size == 2) {
      TreeNode newRoot;
      newRoot.size = 1;
      newRoot.children[0] = add(dummy);
      dummy = newRoot;
    }
    return ret;
  }

  bool erase(const T0::INDEX &val, int tick) {
    bool ret = getRoot().erase(this, {val, tick}, &dummy, 0);
    NodePtr root = getRoot();
    if (!root.isLeaf) {
      TreeNode rootNode = *root.treeNode();
      if (rootNode.size == 1) {
        remove(dummy.children[0]);
        dummy = rootNode;
      }
    }
    return ret;
  }

  iterator find(const T0::INDEX &val) { //find the first element no less than val
    return getRoot().find(this, {val, INT32_MIN}, dummy.children[0]);
  }

  Optional<iterator> get(const T0::INDEX &val, int tick) {
    iterator it = getRoot().find(this, {val, tick}, dummy.children[0]);
    return (!it.end() && it->val.index() == val && it->tick == tick) ? Optional<iterator>(it) : Optional<iterator>();
  }
};

#endif