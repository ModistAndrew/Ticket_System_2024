#ifndef TICKETSYSTEM2024_PRIORITY_QUEUE_HPP
#define TICKETSYSTEM2024_PRIORITY_QUEUE_HPP

#include "Exceptions.hpp"

template<typename T>
class priority_queue {
  static bool defaultCmp(const T &a, const T &b) {
    return a < b;
  }

  decltype(&defaultCmp) compare;

  struct Node {
    T value;
    Node *child, *sibling;

    explicit Node(const T &v) : value(v), child(nullptr), sibling(nullptr) {}

    Node(const Node &other) : value(other.value), child(nullptr), sibling(nullptr) {
      if (other.child) {
        child = new Node(*other.child);
      }
      if (other.sibling) {
        sibling = new Node(*other.sibling);
      }
    }

    ~Node() {
      delete child;
      delete sibling;
    }
  };

  Node *mergeNode(Node *a, Node *b) { // merge two roots and return the new root, a and b will be invalid
    if (!a) {
      return b;
    }
    if (!b) {
      return a;
    }
    if (compare(a->value, b->value)) {
      std::swap(a, b);
    }
    b->sibling = a->child;
    a->child = b;
    return a;
  }

  void compress(Node *&n) { //compress the greatest child to a root. use reference to modify the pointer
    if (!n || !n->sibling) {
      return;
    }
    Node *nxt = n->sibling->sibling;
    n = mergeNode(n, n->sibling);
    n->sibling = nxt;
    Node *two;
    while (n->sibling && n->sibling->sibling) {
      nxt = n->sibling->sibling->sibling;
      two = mergeNode(n->sibling, n->sibling->sibling);
      try {
        n = mergeNode(n, two);
      } catch (...) {
        n->sibling = two;
        two->sibling = nxt;
        throw;
      }
      n->sibling = nxt;
    }
    if (n->sibling) {
      n = mergeNode(n, n->sibling);
      n->sibling = nullptr;
    }
  }

public:
  Node *root;
  size_t length;

  explicit priority_queue(decltype(&defaultCmp) cmp = defaultCmp) : root(nullptr), length(0), compare(cmp) {}

  priority_queue(const priority_queue &other) : root(nullptr), length(other.length), compare(other.compare) {
    if (other.root) {
      root = new Node(*other.root);
    }
  }

  ~priority_queue() {
    delete root;
  }

  priority_queue &operator=(const priority_queue &other) {
    if (this != &other) {
      this->~priority_queue();
      new(this) priority_queue(other);
    }
    return *this;
  }

  const T &top() const {
    if (!root) {
      throw ContainerEmpty();
    }
    return root->value;
  }

  void push(const T &v) {
    Node *newNode = new Node(v);
    try {
      root = mergeNode(root, newNode);
    } catch (...) {
      delete newNode;
      throw;
    }
    length++;
  }

  void pop() {
    if (!root) {
      throw ContainerEmpty();
    }
    compress(root->child);
    Node *old = root;
    root = root->child;
    old->child = nullptr;
    delete old;
    length--;
  }

  size_t size() const {
    return length;
  }

  bool empty() const {
    return length == 0;
  }

  void merge(priority_queue &other) {
    root = mergeNode(root, other.root);
    other.root = nullptr;
    other.length = 0;
    length += other.length;
  }
};

#endif