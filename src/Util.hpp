//
// Created by zjx on 2024/4/26.
//

#ifndef TICKET_SYSTEM_2024_UTIL_HPP
#define TICKET_SYSTEM_2024_UTIL_HPP

#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <cmath>

template<typename T>
T *lower_bound(T *first, T *last, const T &val) {
  while (first < last) {
    T *mid = first + (last - first) / 2;
    if (*mid < val) {
      first = mid + 1;
    } else {
      last = mid;
    }
  }
  return first;
}

template<typename T>
T *upper_bound(T *first, T *last, const T &val) {
  while (first < last) {
    T *mid = first + (last - first) / 2;
    if (val >= *mid) {
      first = mid + 1;
    } else {
      last = mid;
    }
  }
  return first;
}

template<class T1, class T2>
class pair {
public:
  T1 first;
  T2 second;

  constexpr pair() : first(), second() {}

  pair(const pair &other) = default;

  pair(pair &&other) = default;

  pair& operator=(const pair &other) {
    if(this != &other) {
      first = other.first;
      second = other.second;
    }
    return *this;
  }

  pair(const T1 &x, const T2 &y) : first(x), second(y) {}

  template<class U1, class U2>
  pair(U1 &&x, U2 &&y) : first(x), second(y) {}

  template<class U1, class U2>
  explicit pair(const pair<U1, U2> &other) : first(other.first), second(other.second) {}

  template<class U1, class U2>
  explicit pair(pair<U1, U2> &&other) : first(other.first), second(other.second) {}

  auto operator<=>(const pair &) const = default;

  friend std::ostream &operator<<(std::ostream &out, const pair &rhs) {
    out << rhs.first << " " << rhs.second;
    return out;
  }
};

template<int L>
class FixedString { // Fixed length string with max length L
  char key[L];
public:
  explicit FixedString(const std::string &s) : key{} {
    if (s.length() > L) {
      throw;
    }
    strncpy(key, s.c_str(), L);
  }

  FixedString() = default;

  auto operator<=>(const FixedString &) const = default;

  int len() const {
    return strnlen(key, L);
  }

  char *begin() const {
    return key;
  }

  char *end() const {
    return key + len();
  }

  friend std::ostream &operator<<(std::ostream &out, const FixedString &rhs) {
    for (int i = 0; i < L; i++) {
      if (rhs.key[i] == '\0') {
        break;
      }
      out << rhs.key[i];
    }
    return out;
  }
};

#endif //TICKET_SYSTEM_2024_UTIL_HPP
