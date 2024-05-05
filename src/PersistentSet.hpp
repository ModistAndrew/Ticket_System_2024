//
// Created by zjx on 2024/4/22.
//

#ifndef TICKET_SYSTEM_2024_PERSISTENT_SET_HPP
#define TICKET_SYSTEM_2024_PERSISTENT_SET_HPP

#include "Util.hpp"
#include "FileStorage.hpp"

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

    bool operator==(const iterator &rhs) const { //notice that end() == end() is always true
      return leaf == rhs.leaf && pos == rhs.pos;
    }

    bool operator!=(const iterator &rhs) const {
      return leaf != rhs.leaf || pos != rhs.pos;
    }
  };

  class NodePtr {
    int ptr;
  public:
    NodePtr(PersistentSet *set, const TreeNode &treeNode) {
      ptr = set->treeNodeStorage.add(treeNode) << 1 | 0;
    }
    
    NodePtr(PersistentSet *set, const LeafNode &leafNode) {
      ptr = set->leafNodeStorage.add(leafNode) << 1 | 1;
    }
    
    bool isLeaf() {
      return ptr & 1;
    }

    LeafNode leafNode(PersistentSet *set) {
      return set->leafNodeStorage.get(ptr >> 1);
    }

    TreeNode treeNode(PersistentSet *set) {
      return set->treeNodeStorage.get(ptr >> 1);
    }

    bool insert(PersistentSet *set, const T &val, TreeNode* parent, int parentPos) {
      if (isLeaf()) {
        return leafNode(set)->insert(set, val, parent, parentPos);
      } else {
        return treeNode(set)->insert(set, val, parent, parentPos);
      }
    }

    bool erase(PersistentSet *set, const T &val, TreeNode* parent, int parentPos) {
      if(isLeaf()) {
        return leafNode(set)->erase(set, val, parent, parentPos);
      } else {
        return treeNode(set)->erase(set, val, parent, parentPos);
      }
    }

    iterator find(PersistentSet *set, const T &val) {
      if(isLeaf()) {
        return leafNode(set)->find(set, val);
      } else {
        return treeNode(set)->find(set, val);
      }
    }
  };

  struct TreeNode {
    int size = 0; //the number of children
    T index[SIZE_1 - 1];
    NodePtr children[SIZE_1];

    iterator find(PersistentSet *set, const T &val) { //find first no less than val
      int p = upper_bound(index, index + size - 1, val) - index;
      return children[p].find(set, val);
    }

    bool insert(PersistentSet *set, const T &val, TreeNode *parent, int pos) { //insert val into this node
      int p = upper_bound(index, index + size - 1, val) - index;
      NodePtr child = children[p];
      if (child.insert(set, val, this, p)) {
        if (size == SIZE_1) {
          postInsert(set, parent, pos);
        }
        return true;
      }
      return false;
    }

    bool erase(PersistentSet *set, const T &val, TreeNode *parent, int pos) { //erase val from this node
      int p = upper_bound(index, index + size - 1, val) - index;
      NodePtr child = children[p];
      if (child.erase(set, val, this, p)) {
        if (size == SIZE_1 / 2 - 1) {
          postErase(set, parent, pos);
        }
        return true;
      }
      return false;
    }

    void insertChild(NodePtr newChild, const T &newIndex, int pos) { //insert newChild after children[pos] ans newIndex after index[pos-1]
      int indexPos = pos;
      int childPos = pos + 1;
      memmove(index + indexPos + 1, index + indexPos, (size - indexPos - 1) * sizeof(T));
      memmove(children + childPos + 1, children + childPos, (size - childPos) * sizeof(NodePtr));
      index[indexPos] = newIndex;
      children[childPos] = newChild;
      size++;
    }

    void eraseChild(int pos) { //erase a child after children[pos] and index[pos-1]
      int indexPos = pos;
      int childPos = pos + 1;
      memmove(index + indexPos, index + indexPos + 1, (size - indexPos - 2) * sizeof(T));
      memmove(children + childPos, children + childPos + 1, (size - childPos - 1) * sizeof(NodePtr));
      size--;
    }

    void postInsert(PersistentSet *set, TreeNode *parent, int pos) { //when size==SIZE
      TreeNode newNode;
      int half = size / 2;
      newNode.size = half;
      size = half;
      memcpy(newNode.index, index + half, (half - 1) * sizeof(T));
      memcpy(newNode.children, children + half, half * sizeof(NodePtr));
      parent->insertChild(NodePtr(set, newNode), index[half - 1], pos);
    }

    void postErase(PersistentSet *set, TreeNode *parent, int pos) { //when size==SIZE/2-1
      if(parent->size == 1) { //root
        return;
      }
      if (pos == 0) {
        TreeNode sibling = parent->children[pos + 1].treeNode();
        if (sibling.size > SIZE_1 / 2) {
          memcpy(index + size - 1, parent->index + pos, sizeof(T));
          memcpy(children + size, sibling.children, sizeof(NodePtr));
          memcpy(parent->index + pos, sibling.index, sizeof(T));
          memmove(sibling.index, sibling.index + 1, (sibling.size - 2) * sizeof(T));
          memmove(sibling.children, sibling.children + 1, (sibling.size - 1) * sizeof(NodePtr));
          size++;
          sibling.size--;
        } else {
          this->merge(sibling, parent, pos);
        }
      } else {
        TreeNode *sibling = parent->children[pos - 1].treeNode();
        if (sibling->size > SIZE_1 / 2) {
          memmove(index + 1, index, (size - 1) * sizeof(T));
          memmove(children + 1, children, size * sizeof(NodePtr));
          memcpy(index, parent->index + pos - 1, sizeof(T));
          memcpy(children, sibling->children + sibling->size - 1, sizeof(NodePtr));
          memcpy(parent->index + pos - 1, sibling->index + sibling->size - 2, sizeof(T));
          size++;
          sibling->size--;
        } else {
          sibling->merge(this, parent, pos - 1);
        }
      }
    }

    void merge(TreeNode *sibling, TreeNode *parent, int pos) { //sibling is parent->children[pos+1]
      memcpy(index + size - 1, parent->index + pos, sizeof(T));
      memcpy(index + size, sibling->index, (sibling->size - 1) * sizeof(T));
      memcpy(children + size, sibling->children, sibling->size * sizeof(NodePtr));
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
      int p = lower_bound(data, data + size, val) - data;
      return p == size ? iterator(nullptr, 0) : iterator(this, p);
    }

    bool insert(const T &val, TreeNode *parent, int pos) { //insert val into this node
      int p = lower_bound(data, data + size, val) - data;
      if (p < size && data[p] == val) {
        return false;
      }
      memmove(data + p + 1, data + p, (size - p) * sizeof(T));
      data[p] = val;
      size++;
      if (size == SIZE_2) {
        postInsert(parent, pos);
      }
      return true;
    }

    bool erase(const T &val, TreeNode *parent, int pos) { //erase val from this node
      int p = lower_bound(data, data + size, val) - data;
      if (p >= size || data[p] != val) {
        return false;
      }
      memmove(data + p, data + p + 1, (size - p - 1) * sizeof(T));
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
      memcpy(newNode->data, data + half, half * sizeof(T));
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
          memcpy(data + size, sibling->data, sizeof(T)); //copy one here
          memmove(sibling->data, sibling->data + 1, (sibling->size - 1) * sizeof(T)); //delete one from sibling
          memcpy(parent->index + pos, sibling->data, sizeof(T)); //replace parent's index
          size++;
          sibling->size--;
        } else {
          this->merge(sibling, parent, pos);
        }
      } else {
        LeafNode *sibling = parent->children[pos - 1].leafNode();
        if (sibling->size > SIZE_2 / 2) {
          memmove(data, data + 1, size * sizeof(T)); //leave one space for copy
          memcpy(data, sibling->data + sibling->size - 1, sizeof(T)); //copy one here
          memcpy(parent->index + pos - 1, data, sizeof(T)); //replace parent's index
          size++;
          sibling->size--;
        } else {
          sibling->merge(this, parent, pos - 1);
        }
      }
    }

    void merge(LeafNode *sibling, TreeNode *parent, int pos) { //sibling is parent->children[pos+1]
      memcpy(data + size, sibling->data, sibling->size * sizeof(T));
      size += sibling->size;
      next = sibling->next;
      parent->eraseChild(pos);
      delete sibling;
    }
  };

  TreeNode *dummy; //there is a fake tree node which always points to the root
  LeafNode *firstLeaf; //the first leaf node
  FileStorage<TreeNode, NodePtr> treeNodeStorage;
  FileStorage<LeafNode, LeafNode*> leafNodeStorage;

  NodePtr getRoot() {
    return dummy->children[0];
  }
public:
  PersistentSet() {
    dummy = new TreeNode();
    dummy->size = 1;
    root = getPtr(treeNodeStorage.getInfo());
    dummy->children[0] = NodePtr(root);
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