#ifndef TICKETSYSTEM2024_VECTOR_HPP
#define TICKETSYSTEM2024_VECTOR_HPP

#include "../util/Exceptions.hpp"

template<typename T>
class vector {
  const size_t length;
  T *data;

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

  vector(size_t size = 0) : length(size), data(new T[length]) {}

  vector(const vector &other) : length(other.length), data(new T[length]) {
    for (size_t i = 0; i < length; i++) {
      data[i] = other.data[i];
    }
  }

  vector(vector &&other) noexcept : length(other.length), data(other.data) {
    other.data = nullptr;
  }

  vector &operator=(const vector &other) {
    if (this != &other) {
      this->~vector();
      new(this) vector(other);
    }
    return *this;
  }

  vector &operator=(vector &&other) noexcept {
    if (this != &other) {
      this->~vector();
      new(this) vector(other);
    }
    return *this;
  }

  ~vector() {
    delete[] data;
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
};

#endif