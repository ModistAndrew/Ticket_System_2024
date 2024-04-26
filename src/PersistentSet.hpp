//
// Created by zjx on 2024/4/22.
//

#ifndef TICKET_SYSTEM_2024_PERSISTENT_SET_HPP
#define TICKET_SYSTEM_2024_PERSISTENT_SET_HPP

#include <bits/stdc++.h>

template<typename T, int SIZE_1, int SIZE_2>
class PersistentSet {
  static_assert(SIZE_1 >= 4 && SIZE_1 % 2 == 0, "SIZE_1 must be even and at least 4");
  static_assert(SIZE_2 >= 4 && SIZE_2 % 2 == 0, "SIZE_2 must be even and at least 4");

  struct TreeNode;
  struct LeafNode;

  class iterator {
    LeafNode *leaf;
    int pos;
  public:
    iterator(LeafNode *leaf, int pos) : leaf(leaf), pos(pos) {}

    iterator &operator++() {
      if(leaf == nullptr) {
        throw;
      }
      pos++;
      if (pos == leaf->size) {
        leaf = leaf->next;
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
      if(leaf == nullptr) {
        throw;
      }
      return leaf->data[pos];
    }

    bool operator==(const iterator &rhs) const { //notice that end() == end() is true
      return leaf == rhs.leaf && pos == rhs.pos;
    }

    bool operator!=(const iterator &rhs) const {
      return leaf != rhs.leaf || pos != rhs.pos;
    }
  };

  class NodePtr {
    std::any ptr;
  public:
    bool isLeaf;
    explicit NodePtr(TreeNode *ptr) : ptr(ptr), isLeaf(false) {}

    explicit NodePtr(LeafNode *ptr) : ptr(ptr), isLeaf(true) {}

    NodePtr() : ptr(nullptr), isLeaf(false) {}

    TreeNode *treeNode() {
      return std::any_cast<TreeNode *>(ptr);
    }

    LeafNode *leafNode() {
      return std::any_cast<LeafNode *>(ptr);
    }

    bool insert(const T &val, TreeNode* parent, int parentPos) {
      if (isLeaf) {
        return leafNode()->insert(val, parent, parentPos);
      } else {
        return treeNode()->insert(val, parent, parentPos);
      }
    }

    bool erase(const T &val, TreeNode* parent, int parentPos) {
      if (isLeaf) {
        return leafNode()->erase(val, parent, parentPos);
      } else {
        return treeNode()->erase(val, parent, parentPos);
      }
    }

    iterator find(const T &val) {
      if (isLeaf) {
        return leafNode()->find(val);
      } else {
        return treeNode()->find(val);
      }
    }
  };

  struct TreeNode {
    int size = 0; //the number of children
    T index[SIZE_1 - 1];
    NodePtr children[SIZE_1];

    iterator find(const T &val) { //find first no less than val
      int p = std::upper_bound(index, index + size - 1, val) - index;
      return children[p].find(val);
    }

    bool insert(const T &val, TreeNode *parent, int pos) { //insert val into this node
      int p = std::upper_bound(index, index + size - 1, val) - index;
      NodePtr child = children[p];
      if (child.insert(val, this, p)) {
        if (size == SIZE_1) {
          postInsert(parent, pos);
        }
        return true;
      }
      return false;
    }

    bool erase(const T &val, TreeNode *parent, int pos) { //erase val from this node
      int p = std::upper_bound(index, index + size - 1, val) - index;
      NodePtr child = children[p];
      if (child.erase(val, this, p)) {
        if (size == SIZE_1 / 2 - 1) {
          postErase(parent, pos);
        }
        return true;
      }
      return false;
    }

    void insertChild(NodePtr newChild, const T &newIndex, int pos) { //insert newChild after children[pos] ans newIndex after index[pos-1]
      int indexPos = pos;
      int childPos = pos + 1;
      std::memmove(index + indexPos + 1, index + indexPos, (size - indexPos - 1) * sizeof(T));
      std::memmove(children + childPos + 1, children + childPos, (size - childPos) * sizeof(NodePtr));
      index[indexPos] = newIndex;
      children[childPos] = newChild;
      size++;
    }

    void eraseChild(int pos) { //erase a child after children[pos] and index[pos-1]
      int indexPos = pos;
      int childPos = pos + 1;
      std::memmove(index + indexPos, index + indexPos + 1, (size - indexPos - 2) * sizeof(T));
      std::memmove(children + childPos, children + childPos + 1, (size - childPos - 1) * sizeof(NodePtr));
      size--;
    }

    void postInsert(TreeNode *parent, int pos) { //when size==SIZE
      auto *newNode = new TreeNode();
      int half = size / 2;
      newNode->size = half;
      size = half;
      std::memcpy(newNode->index, index + half, (half - 1) * sizeof(T));
      std::memcpy(newNode->children, children + half, half * sizeof(NodePtr));
      parent->insertChild(NodePtr(newNode), index[half - 1], pos);
    }

    void postErase(TreeNode *parent, int pos) { //when size==SIZE/2-1
      if(parent->size == 1) { //root
        return;
      }
      if (pos == 0) {
        TreeNode *sibling = parent->children[pos + 1].treeNode();
        if (sibling->size > SIZE_1 / 2) {
          std::memcpy(index + size - 1, parent->index + pos, sizeof(T));
          std::memcpy(children + size, sibling->children, sizeof(NodePtr));
          std::memcpy(parent->index + pos, sibling->index, sizeof(T));
          std::memmove(sibling->index, sibling->index + 1, (sibling->size - 2) * sizeof(T));
          std::memmove(sibling->children, sibling->children + 1, (sibling->size - 1) * sizeof(NodePtr));
          size++;
          sibling->size--;
        } else {
          this->merge(sibling, parent, pos);
        }
      } else {
        TreeNode *sibling = parent->children[pos - 1].treeNode();
        if (sibling->size > SIZE_1 / 2) {
          std::memmove(index + 1, index, (size - 1) * sizeof(T));
          std::memmove(children + 1, children, size * sizeof(NodePtr));
          std::memcpy(index, parent->index + pos - 1, sizeof(T));
          std::memcpy(children, sibling->children + sibling->size - 1, sizeof(NodePtr));
          std::memcpy(parent->index + pos - 1, sibling->index + sibling->size - 2, sizeof(T));
          size++;
          sibling->size--;
        } else {
          sibling->merge(this, parent, pos - 1);
        }
      }
    }

    void merge(TreeNode *sibling, TreeNode *parent, int pos) { //sibling is parent->children[pos+1]
      std::memcpy(index + size - 1, parent->index + pos, sizeof(T));
      std::memcpy(index + size, sibling->index, (sibling->size - 1) * sizeof(T));
      std::memcpy(children + size, sibling->children, sibling->size * sizeof(NodePtr));
      size += sibling->size;
      parent->eraseChild(pos);
      delete sibling;
    }
  };

  struct LeafNode {
    int size = 0;
    T data[SIZE_2];
    LeafNode *next; //linked list

    iterator find(const T &val) { //find first no less than val
      int p = std::lower_bound(data, data + size, val) - data;
      return p == size ? iterator(nullptr, 0) : iterator(this, p);
    }

    bool insert(const T &val, TreeNode *parent, int pos) { //insert val into this node
      int p = std::lower_bound(data, data + size, val) - data;
      if (p < size && data[p] == val) {
        return false;
      }
      std::memmove(data + p + 1, data + p, (size - p) * sizeof(T));
      data[p] = val;
      size++;
      if (size == SIZE_2) {
        postInsert(parent, pos);
      }
      return true;
    }

    bool erase(const T &val, TreeNode *parent, int pos) { //erase val from this node
      int p = std::lower_bound(data, data + size, val) - data;
      if (p >= size || data[p] != val) {
        return false;
      }
      std::memmove(data + p, data + p + 1, (size - p - 1) * sizeof(T));
      size--;
      if (size == SIZE_2 / 2 - 1) {
        postErase(parent, pos);
      }
      return true;
    }

    void postInsert(TreeNode *parent, int pos) { //when size==SIZE
      auto *newNode = new LeafNode();
      int half = size / 2;
      newNode->size = half;
      size = half;
      std::memcpy(newNode->data, data + half, half * sizeof(T));
      newNode->next = next;
      next = newNode;
      parent->insertChild(NodePtr(newNode), data[half], pos);
    }

    void postErase(TreeNode *parent, int pos) { //when size==SIZE/2-1
      if(parent->size == 1) { //root
        return;
      }
      if (pos == 0) {
        LeafNode *sibling = parent->children[pos + 1].leafNode();
        if (sibling->size > SIZE_2 / 2) {
          std::memcpy(data + size, sibling->data, sizeof(T)); //copy one here
          std::memmove(sibling->data, sibling->data + 1, (sibling->size - 1) * sizeof(T)); //delete one from sibling
          std::memcpy(parent->index + pos, sibling->data, sizeof(T)); //replace parent's index
          size++;
          sibling->size--;
        } else {
          this->merge(sibling, parent, pos);
        }
      } else {
        LeafNode *sibling = parent->children[pos - 1].leafNode();
        if (sibling->size > SIZE_2 / 2) {
          std::memmove(data, data + 1, size * sizeof(T)); //leave one space for copy
          std::memcpy(data, sibling->data + sibling->size - 1, sizeof(T)); //copy one here
          std::memcpy(parent->index + pos - 1, data, sizeof(T)); //replace parent's index
          size++;
          sibling->size--;
        } else {
          sibling->merge(this, parent, pos - 1);
        }
      }
    }

    void merge(LeafNode *sibling, TreeNode *parent, int pos) { //sibling is parent->children[pos+1]
      std::memcpy(data + size, sibling->data, sibling->size * sizeof(T));
      size += sibling->size;
      next = sibling->next;
      parent->eraseChild(pos);
      delete sibling;
    }
  };

  TreeNode *dummy; //there a fake tree node which always points to the root

  NodePtr getRoot() {
    return dummy->children[0];
  }
public:
  PersistentSet() {
    dummy = new TreeNode();
    dummy->size = 1;
    dummy->children[0] = NodePtr(new LeafNode());
  }

  bool insert(const T &val) {
    bool ret = getRoot().insert(val, dummy, 0);
    if(dummy->size == 2) {
      auto *newRoot = new TreeNode();
      newRoot->size = 1;
      newRoot->children[0] = NodePtr(dummy);
      dummy = newRoot;
    }
    return ret;
  }

  bool erase(const T &val) {
    bool ret = getRoot().erase(val, dummy, 0);
    NodePtr root = getRoot();
    if(!root.isLeaf) {
      TreeNode *rootNode = root.treeNode();
      if(rootNode->size == 1) {
        NodePtr newDummy = root;
        delete dummy;
        dummy = newDummy.treeNode();
      }
    }
    return ret;
  }

  iterator find(const T &val) {
    return getRoot().find(val);
  }
};

#endif //TICKET_SYSTEM_2024_PERSISTENT_SET_HPP