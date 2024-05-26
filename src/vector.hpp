#ifndef SJTU_VECTOR_HPP
#define SJTU_VECTOR_HPP

#include "Exceptions.hpp"

template<typename T>
class vector {
  T *data;
  size_t length;
  size_t capacity;

  void resize(size_t newSize) {
    data = (T *) realloc(data, newSize * sizeof(T));
    capacity = newSize;
  }

public:
  class const_iterator;

  class iterator {
    friend class vector;

    const vector *vec;
    T *ptr;
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T *;
    using reference = T &;
    using iterator_category = std::output_iterator_tag;

    iterator(const vector *vec, T *ptr) : vec(vec), ptr(ptr) {}

    iterator operator+(const int &n) const {
      return iterator{vec, ptr + n};
    }

    iterator operator-(const int &n) const {
      return iterator{vec, ptr - n};
    }

    int operator-(const iterator &rhs) const {
      if (vec != rhs.vec) {
        throw InvalidIterator();
      }
      return ptr - rhs.ptr;
    }

    iterator &operator+=(const int &n) {
      *this = *this + n;
      return *this;
    }

    iterator &operator-=(const int &n) {
      *this = *this - n;
      return *this;
    }

    iterator operator++(int) {
      iterator ret = *this;
      ptr++;
      return ret;
    }

    iterator &operator++() {
      ptr++;
      return *this;
    }

    iterator operator--(int) {
      iterator ret = *this;
      ptr--;
      return ret;
    }

    iterator &operator--() {
      ptr--;
      return *this;
    }

    T &operator*() const {
      return *ptr;
    }

    bool operator==(const iterator &rhs) const {
      return ptr == rhs.ptr;
    }

    bool operator==(const const_iterator &rhs) const {
      return ptr == rhs.ptr;
    }

    bool operator!=(const iterator &rhs) const {
      return ptr != rhs.ptr;
    }

    bool operator!=(const const_iterator &rhs) const {
      return ptr != rhs.ptr;
    }
  };

  class const_iterator {
    friend class vector;

    const vector *vec;
    const T *ptr;
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T *;
    using reference = T &;
    using iterator_category = std::output_iterator_tag;

    const_iterator(const vector *vec, const T *ptr) : vec(vec), ptr(ptr) {}

    const_iterator operator+(const int &n) const {
      return const_iterator{vec, ptr + n};
    }

    const_iterator operator-(const int &n) const {
      return const_iterator{vec, ptr - n};
    }

    int operator-(const const_iterator &rhs) const {
      if (vec != rhs.vec) {
        throw InvalidIterator();
      }
      return ptr - rhs.ptr;
    }

    const_iterator &operator+=(const int &n) {
      *this = *this + n;
      return *this;
    }

    const_iterator &operator-=(const int &n) {
      *this = *this - n;
      return *this;
    }

    const_iterator operator++(int) {
      const_iterator ret = *this;
      ptr++;
      return ret;
    }

    const_iterator &operator++() {
      ptr++;
      return *this;
    }

    const_iterator operator--(int) {
      const_iterator ret = *this;
      ptr--;
      return ret;
    }

    const_iterator &operator--() {
      ptr--;
      return *this;
    }

    const T &operator*() const {
      return *ptr;
    }

    bool operator==(const iterator &rhs) const {
      return ptr == rhs.ptr;
    }

    bool operator==(const const_iterator &rhs) const {
      return ptr == rhs.ptr;
    }

    bool operator!=(const iterator &rhs) const {
      return ptr != rhs.ptr;
    }

    bool operator!=(const const_iterator &rhs) const {
      return ptr != rhs.ptr;
    }
  };

  vector() : length(), capacity(), data() {
    resize(1);
  }

  vector(const vector &other) {
    length = other.length;
    capacity = other.capacity;
    data = (T *) malloc(capacity * sizeof(T));
    for (size_t i = 0; i < length; i++) {
      new(data + i) T(other.data[i]);
    }
  }

  ~vector() {
    for (size_t i = 0; i < length; i++) {
      data[i].~T();
    }
    free(data);
  }

  vector &operator=(const vector &other) {
    if (this != &other) {
      this->~vector();
      new(this) vector(other);
    }
    return *this;
  }

  T &at(const size_t &pos) {
    if (pos >= length) {
      throw IndexOutOfBound();
    }
    return data[pos];
  }

  const T &at(const size_t &pos) const {
    if (pos >= length) {
      throw IndexOutOfBound();
    }
    return data[pos];
  }

  T &operator[](const size_t &pos) {
    if (pos >= length) {
      throw IndexOutOfBound();
    }
    return data[pos];
  }

  const T &operator[](const size_t &pos) const {
    if (pos >= length) {
      throw IndexOutOfBound();
    }
    return data[pos];
  }

  const T &front() const {
    if (length == 0) {
      throw ContainerEmpty();
    }
    return data[0];
  }

  const T &back() const {
    if (length == 0) {
      throw ContainerEmpty();
    }
    return data[length - 1];
  }

  iterator begin() {
    return iterator{this, data};
  }

  const_iterator cbegin() const {
    return const_iterator{this, data};
  }

  iterator end() {
    return iterator{this, data + length};
  }

  const_iterator cend() const {
    return const_iterator{this, data + length};
  }

  bool empty() const {
    return length == 0;
  }

  size_t size() const {
    return length;
  }

  void clear() {
    this->~vector();
    new(this) vector();
  }

  iterator insert(iterator pos, const T &value) {
    if (length - (pos.ptr - data) > 0) {
      memmove(pos.ptr + 1, pos.ptr, (length - (pos.ptr - data)) * sizeof(T));
    }
    new(pos.ptr) T(value);
    length++;
    if (length == capacity) {
      resize(capacity * 2);
    }
    return pos;
  }

  iterator insert(const size_t &ind, const T &value) {
    if (ind > length) {
      throw IndexOutOfBound();
    }
    return insert(begin() + ind, value);
  }

  iterator erase(iterator pos) {
    pos.ptr->~T();
    if (length - (pos.ptr - data) - 1 > 0) {
      memmove(pos.ptr, pos.ptr + 1, (length - (pos.ptr - data) - 1) * sizeof(T));
    }
    length--;
    if (length * 4 <= capacity) {
      resize(capacity / 2);
    }
    return pos;
  }

  iterator erase(const size_t &ind) {
    if (ind >= length) {
      throw IndexOutOfBound();
    }
    return erase(begin() + ind);
  }

  void push_back(const T &value) {
    insert(end(), value);
  }

  void pop_back() {
    if (length == 0) {
      throw ContainerEmpty();
    }
    erase(end() - 1);
  }
};

#endif