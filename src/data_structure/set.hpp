#ifndef TICKETSYSTEM2024_SET_HPP
#define TICKETSYSTEM2024_SET_HPP

#include "../util/Exceptions.hpp"
#include "pair.hpp"

template<class T, class Compare = std::less<T>>
class set {
public:
  using value_type = T;
private:
  enum Color {
    DUMMY, RED, BLACK
  };

  //nothing should be called on dummy!
  struct Node {
    value_type data;
    Node *left, *right, *parent;
    Color color;

    explicit Node(const value_type &data) : data(data), left(nullptr), right(nullptr), parent(nullptr),
                                            color(RED) {}

    Node(const Node &other) : data(other.data), left(other.left), right(other.right), parent(other.parent),
                              color(other.color) {}

    static bool root(const Node *p) {
      return p->parent->color == DUMMY;
    }

    static bool red(const Node *p) {
      return p && p->color == RED;
    }

    static bool black(const Node *p) {
      return !p || p->color == BLACK;
    }

    Node *leftParent() const {
      if (root(this)) {
        return nullptr;
      }
      return parent->right == this ? parent : nullptr;
    }

    Node *rightParent() const {
      if (root(this)) {
        return nullptr;
      }
      return parent->left == this ? parent : nullptr;
    }

    //let left child be here
    //must have left child
    void leftUp() {
      Node *l = left;
      replaceWith(l);
      setLeft(l->right);
      l->setRight(this);
    }

    //let right child be here
    //must have right child
    void rightUp() {
      Node *r = right;
      replaceWith(r);
      setRight(r->left);
      r->setLeft(this);
    }

    Node *max() {
      Node *cur = this;
      while (cur->right) {
        cur = cur->right;
      }
      return cur;
    }

    Node *min() {
      Node *cur = this;
      while (cur->left) {
        cur = cur->left;
      }
      return cur;
    }

    Node *next() const {
      if (right) {
        return right->min();
      }
      const Node *cur = this;
      while (cur->leftParent()) {
        cur = cur->parent;
      }
      return cur->rightParent(); //may be nullptr
    }

    Node *pre() const {
      if (left) {
        return left->max();
      }
      const Node *cur = this;
      while (cur->rightParent()) {
        cur = cur->parent;
      }
      return cur->leftParent(); //may be nullptr
    }

    //replace left with p(may be nullptr)
    //notice that p's original parent still points to p; self's origin child still points to self
    void setLeft(Node *p) {
      left = p;
      if (p) {
        p->parent = this;
      }
    }

    //replace right with p(may be nullptr)
    //notice that p's original parent still points to p; self's origin child still points to self
    void setRight(Node *p) {
      right = p;
      if (p) {
        p->parent = this;
      }
    }

    //set parent's child to p(may be nullptr)
    //self may be root
    void replaceWith(Node *p) {
      if (leftParent()) {
        parent->setRight(p);
      } else { //self is root or right child
        parent->setLeft(p);
      }
    }

    //used when self is inserted. self is red
    void checkInsert() {
      if (root(this)) {
        color = BLACK;
        return;
      }
      if (black(parent)) {
        return;
      }
      //must have grandparent
      if (rightParent()) {
        if (parent->rightParent()) {
          if (red(parent->parent->right)) {
            parent->color = BLACK;
            parent->parent->right->color = BLACK;
            parent->parent->color = RED;
            parent->parent->checkInsert();
          } else {
            parent->parent->leftUp();
            parent->color = BLACK;
            parent->right->color = RED;
          }
        } else {
          if (red(parent->parent->left)) {
            parent->color = BLACK;
            parent->parent->left->color = BLACK;
            parent->parent->color = RED;
            parent->parent->checkInsert();
          } else {
            parent->leftUp();
            parent->rightUp();
            color = BLACK;
            left->color = RED;
          }
        }
      } else {
        if (parent->leftParent()) {
          if (red(parent->parent->left)) {
            parent->color = BLACK;
            parent->parent->left->color = BLACK;
            parent->parent->color = RED;
            parent->parent->checkInsert();
          } else {
            parent->parent->rightUp();
            parent->color = BLACK;
            parent->left->color = RED;
          }
        } else {
          if (red(parent->parent->right)) {
            parent->color = BLACK;
            parent->parent->right->color = BLACK;
            parent->parent->color = RED;
            parent->parent->checkInsert();
          } else {
            parent->rightUp();
            parent->leftUp();
            color = BLACK;
            right->color = RED;
          }
        }
      }
    }

    //make self deeper(containing an extra black node) for deletion
    //self is black
    void moreBlack() {
      if (root(this)) {
        return;
      }
      //sibling must exist
      if (rightParent()) {
        if (red(parent->right)) {
          parent->rightUp();
          parent->color = RED;
          parent->parent->color = BLACK;
        }
        if (red(parent->right->left)) {
          parent->right->leftUp();
          parent->rightUp();
          parent->parent->color = parent->color;
          parent->color = BLACK;
        } else if (red(parent->right->right)) {
          parent->rightUp();
          parent->parent->right->color = parent->color;
        } else {
          parent->right->color = RED;
          if (black(parent)) {
            parent->moreBlack();
          } else {
            parent->color = BLACK;
          }
        }
      } else {
        if (red(parent->left)) {
          parent->leftUp();
          parent->color = RED;
          parent->parent->color = BLACK;
        }
        if (red(parent->left->right)) {
          parent->left->rightUp();
          parent->leftUp();
          parent->parent->color = parent->color;
          parent->color = BLACK;
        } else if (red(parent->left->left)) {
          parent->leftUp();
          parent->parent->left->color = parent->color;
        } else {
          parent->left->color = RED;
          if (black(parent)) {
            parent->moreBlack();
          } else {
            parent->color = BLACK;
          }
        }
      }
    }

    //used to erase self. self has at most one child
    void balanceAndErase() {
      if (left) {
        if (red(left)) {
          left->color = BLACK;
        }
        replaceWith(left);
      } else if (right) {
        if (red(right)) {
          right->color = BLACK;
        }
        replaceWith(right);
      } else { //leaf
        if (black(this)) {
          moreBlack();
        }
        replaceWith(nullptr);
      }
    }

    //clear self and its children
    void clear() {
      if (left) {
        left->clear();
      }
      if (right) {
        right->clear();
      }
      delete this;
    }

    //copy self and its children
    Node *copy() {
      Node *newNode = new Node(*this);
      if (left) {
        newNode->setLeft(left->copy());
      }
      if (right) {
        newNode->setRight(right->copy());
      }
      return newNode;
    }
  };

  Node *dummy;
  int length;

  Node *root() const {
    return dummy->left;
  }

public:
  class const_iterator;

  class iterator {
    friend class set;

    set *m;
    Node *ptr; //if ptr == nullptr, it means the iterator is m.end(). may be invalidated by erase!
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = value_type *;
    using reference = value_type &;
    using iterator_category = std::bidirectional_iterator_tag;

    iterator() = default;

    iterator(set *m, Node *ptr) : m(m), ptr(ptr) {}

    iterator(const iterator &other) : m(other.m), ptr(other.ptr) {}

    iterator operator++(int) {
      iterator ret = *this;
      ++*this;
      return ret;
    }

    iterator &operator++() {
      if (!ptr) { //end()++
        throw InvalidIterator();
      }
      ptr = ptr->next();
      return *this;
    }

    iterator operator--(int) {
      iterator ret = *this;
      --*this;
      return ret;
    }

    iterator &operator--() {
      if (!ptr) { //end()--
        if (m->empty()) { //empty
          throw InvalidIterator();
        }
        ptr = m->root()->max();
      } else {
        ptr = ptr->pre();
        if (!ptr) { //begin()--
          throw InvalidIterator();
        }
      }
      return *this;
    }

    value_type &operator*() const {
      if (!ptr) {
        throw InvalidIterator();
      }
      return ptr->data;
    }

    bool operator==(const iterator &rhs) const {
      return ptr == rhs.ptr && m == rhs.m;
    }

    bool operator==(const const_iterator &rhs) const {
      return ptr == rhs.ptr && m == rhs.m;
    }

    bool operator!=(const iterator &rhs) const {
      return ptr != rhs.ptr || m != rhs.m;
    }

    bool operator!=(const const_iterator &rhs) const {
      return ptr != rhs.ptr || m != rhs.m;
    }

    value_type *operator->() const noexcept {
      if (!ptr) {
        throw InvalidIterator();
      }
      return &ptr->data;
    }
  };

  class const_iterator {
    friend class set;

    const set *m;
    const Node *ptr; //if ptr == nullptr, it means the iterator is m.end(). may be invalidated by erase!
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = value_type *;
    using reference = value_type &;
    using iterator_category = std::bidirectional_iterator_tag;

    const_iterator() = default;

    const_iterator(const set *m, const Node *ptr) : m(m), ptr(ptr) {}

    const_iterator(const iterator &other) : m(other.m), ptr(other.ptr) {}

    const_iterator(const const_iterator &other) : m(other.m), ptr(other.ptr) {}

    const_iterator operator++(int) {
      const_iterator ret = *this;
      ++*this;
      return ret;
    }

    const_iterator &operator++() {
      if (!ptr) { //end()++
        throw InvalidIterator();
      }
      ptr = ptr->next();
      return *this;
    }

    const_iterator operator--(int) {
      const_iterator ret = *this;
      --*this;
      return ret;
    }

    const_iterator &operator--() {
      if (!ptr) { //end()--
        if (m->empty()) { //empty
          throw InvalidIterator();
        }
        ptr = m->root()->max();
      } else {
        ptr = ptr->pre();
        if (!ptr) { //begin()--
          throw InvalidIterator();
        }
      }
      return *this;
    }

    const value_type &operator*() const {
      if (!ptr) {
        throw InvalidIterator();
      }
      return ptr->data;
    }

    bool operator==(const iterator &rhs) const {
      return ptr == rhs.ptr && m == rhs.m;
    }

    bool operator==(const const_iterator &rhs) const {
      return ptr == rhs.ptr && m == rhs.m;
    }

    bool operator!=(const iterator &rhs) const {
      return ptr != rhs.ptr || m != rhs.m;
    }

    bool operator!=(const const_iterator &rhs) const {
      return ptr != rhs.ptr || m != rhs.m;
    }

    const value_type *operator->() const noexcept {
      if (!ptr) {
        throw InvalidIterator();
      }
      return &ptr->data;
    }
  };

  set() : length(0) {
    dummy = (Node *) malloc(sizeof(Node));
    dummy->color = DUMMY;
    dummy->left = nullptr;
    dummy->right = nullptr;
    dummy->parent = nullptr;
  }

  set(const set &other) : set() {
    if (other.root()) {
      dummy->setLeft(other.root()->copy());
    }
    length = other.length;
  }

  set &operator=(const set &other) {
    if (this != &other) {
      this->~set();
      new(this) set(other);
    }
    return *this;
  }

  ~set() {
    if (root()) {
      root()->clear();
    }
    free(dummy);
  }

  iterator begin() {
    return iterator(this, root() ? root()->min() : nullptr);
  }

  const_iterator cbegin() const {
    return const_iterator(this, root() ? root()->min() : nullptr);
  }

  iterator end() {
    return iterator(this, nullptr);
  }

  const_iterator cend() const {
    return const_iterator(this, nullptr);
  }

  bool empty() const {
    return length == 0;
  }

  size_t size() const {
    return length;
  }

  void clear() {
    *this = set();
  }

  pair<iterator, bool> insert(const value_type &value) {
    if (!root()) {
      Node *newNode = new Node(value);
      dummy->setLeft(newNode);
      newNode->checkInsert();
      length++;
      return pair<iterator, bool>(iterator(this, newNode), true);
    }
    Node *cur = root();
    while (true) {
      if (Compare()(value, cur->data)) {
        if (!cur->left) {
          Node *newNode = new Node(value);
          cur->setLeft(newNode);
          newNode->checkInsert();
          length++;
          return pair<iterator, bool>(iterator(this, newNode), true);
        }
        cur = cur->left;
      } else if (Compare()(cur->data, value)) {
        if (!cur->right) {
          Node *newNode = new Node(value);
          cur->setRight(newNode);
          newNode->checkInsert();
          length++;
          return pair<iterator, bool>(iterator(this, newNode), true);
        }
        cur = cur->right;
      } else {
        return pair<iterator, bool>(iterator(this, cur), false);
      }
    }
  }


  iterator lower_bound(const value_type &value) {
    Node *cur = root();
    Node *ret = nullptr; // Node to store the result
    while (cur) {
      if (!Compare()(cur->data, value)) { // cur->data >= value
        ret = cur; // Update the result
        cur = cur->left; // Continue searching in the left subtree
      } else {
        cur = cur->right; // Continue searching in the right subtree
      }
    }
    if (!ret) {
      return end();
    }
    return iterator(this, ret);
  }
};

#endif
