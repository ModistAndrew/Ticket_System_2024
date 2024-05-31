//
// Created by zjx on 2024/4/26.
//

#ifndef TICKETSYSTEM2024_UTIL_HPP
#define TICKETSYSTEM2024_UTIL_HPP

#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <cmath>
#include "StringParser.hpp"
#include "map.hpp"
#include "priority_queue.hpp"
#include "list.hpp"

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

template<typename T, typename INDEX>
T *lower_index_bound(T *first, T *last, const INDEX &val) {
  while (first < last) {
    T *mid = first + (last - first) / 2;
    if (mid->index() < val) {
      first = mid + 1;
    } else {
      last = mid;
    }
  }
  return first;
}

template<typename T, typename INDEX>
T *upper_index_bound(T *first, T *last, const INDEX &val) {
  while (first < last) {
    T *mid = first + (last - first) / 2;
    if (val >= mid->index()) {
      first = mid + 1;
    } else {
      last = mid;
    }
  }
  return first;
}

struct Chrono {
  int date;
  int time;

  Chrono() : date(0), time(-1) {}

  Chrono(int d, int t) : date(d + t / 1440), time(t % 1440) {}

  friend std::ostream &operator<<(std::ostream &out, const Chrono &rhs) {
    if (rhs.time < 0) {
      out << "xx-xx xx:xx";
    } else {
      out << toStringDate(rhs.date) << ' ' << toStringTime(rhs.time);
    }
    return out;
  }

  int toTick() const {
    return date * 1440 + time;
  }

};

template<typename T>
struct Optional {
  T value;
  bool present;

  Optional() : present(false) {}

  Optional(const T &value) : value(value), present(true) {}

  Optional(T&& value) : value(value), present(true) {}
};

template<int L>
class FixedString { // Fixed length string with max length L
  char key[L];
public:
  FixedString(const std::string &s) : key{} { //use implicit conversion
    if (s.length() > L) {
      throw;
    }
    strncpy(key, s.c_str(), L);
  }
  
  FixedString() = default;
  
  auto operator<=>(const FixedString &rhs) const = default;

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

  bool empty() const {
    return key[0] == '\0';
  }
};

using String20 = FixedString<20>;
using String30 = FixedString<30>;
using String40 = FixedString<40>;

#endif
