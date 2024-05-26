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
#include <vector>
using std::vector;

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
    if (mid->index < val) {
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
    if (val >= mid->index) {
      first = mid + 1;
    } else {
      last = mid;
    }
  }
  return first;
}

int parseInt(const std::string &s) {
  int x = 0;
  bool neg = false;
  for (char c: s) {
    if (c == '-') {
      neg = true;
      continue;
    }
    x = x * 10 + c - '0';
  }
  return neg ? -x : x;
}

std::string toStringInt(int x) {
  if (x == 0) {
    return "0";
  }
  std::string ret;
  if (x < 0) {
    ret.push_back('-');
    x = -x;
  }
  std::string tmp;
  while (x) {
    tmp.push_back(x % 10 + '0');
    x /= 10;
  }
  for (int i = tmp.length() - 1; i >= 0; i--) {
    ret.push_back(tmp[i]);
  }
  return ret;
}

//vector shouldn't be empty and there should be no empty string
vector<std::string> parseVector(const std::string &s, char delim) {
  vector<std::string> ret;
  std::string tmp;
  for (char c: s) {
    if (c == delim) {
      ret.push_back(tmp); //we accept empty string
      tmp.clear();
    } else {
      tmp.push_back(c);
    }
  }
  ret.push_back(tmp);
  return ret;
}

std::string toStringVector(const vector<std::string> &v, char delim) {
  std::string ret;
  for (int i = 0; i < v.size(); i++) {
    if (i) {
      ret += delim;
    }
    ret += v[i];
  }
  return ret;
}

//use "_" to represent empty vector
vector<int> parseIntVector(const std::string &s, char delim) {
  if(s=="_") {
    return {};
  }
  vector<int> ret;
  std::string tmp;
  for (char c: s) {
    if (c == delim) {
      ret.push_back(parseInt(tmp));
      tmp.clear();
    } else {
      tmp.push_back(c);
    }
  }
  ret.push_back(parseInt(tmp));
  return ret;
}

std::string toStringIntVector(const vector<int> &v, char delim) {
  if(v.empty()) {
    return "_";
  }
  std::string ret;
  for (int i = 0; i < v.size(); i++) {
    if (i) {
      ret += delim;
    }
    ret += toStringInt(v[i]);
  }
  return ret;
}

int parseTime(const std::string &s) { //00:00 to 23:59
  return ((s[0] - '0') * 10 + (s[1] - '0')) * 60 + ((s[3] - '0') * 10 + (s[4] - '0'));
}

std::string toStringTime(int x) {
  std::string ret;
  int hour = x / 60;
  int minute = x % 60;
  ret.push_back(hour / 10 + '0');
  ret.push_back(hour % 10 + '0');
  ret.push_back(':');
  ret.push_back(minute / 10 + '0');
  ret.push_back(minute % 10 + '0');
  return ret;
}

int parseDate(const std::string &s) { //06-01 to 08-31
  int day = (s[3] - '0') * 10 + (s[4] - '0');
  if (s[1] == '6') {
    return day - 1;
  } else if (s[1] == '7') {
    return day + 29;
  } else {
    return day + 60;
  }
}

std::string toStringDate(int x) {
  std::string ret;
  if (x < 30) {
    ret = "06-";
    x += 1;
  } else if (x < 61) {
    ret = "07-";
    x -= 29;
  } else {
    ret = "08-";
    x -= 60;
  }
  ret.push_back(x / 10 + '0');
  ret.push_back(x % 10 + '0');
  return ret;
}

template<class T1, class T2>
struct pair {
  T1 first;
  T2 second;

  constexpr pair() : first(), second() {}

  pair(const pair &other) = default;

  pair(pair &&other) = default;

  pair &operator=(const pair &other) {
    if (this != &other) {
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

template<typename T>
struct Optional {
  T value;
  bool present;

  Optional() : present(false) {}

  Optional(const T &value) : value(value), present(true) {}
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

  bool operator==(const FixedString &rhs) const = default;

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
#endif //TICKET_SYSTEM_2024_UTIL_HPP
