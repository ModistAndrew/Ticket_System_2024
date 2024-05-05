//
// Created by zjx on 2024/4/26.
//
#include <bits/stdc++.h>
#include "PersistentSet.hpp"

template<int L>
class FixedString { // Fixed length string with max length L
  char key[L];
public:
  FixedString(const std::string &s) : key{} {
    if (s.length() > L) {
      throw;
    }
    strncpy(key, s.c_str(), L);
  }

  FixedString() = default;

  bool operator<(const FixedString &rhs) const {
    int result = std::strncmp(key, rhs.key, L);
    return result < 0;
  }

  bool operator==(const FixedString &rhs) const {
    return std::strncmp(key, rhs.key, L) == 0;
  }

  [[nodiscard]] int len() const {
    return strnlen(key, L);
  }

  [[nodiscard]] char *begin() const {
    return key;
  }

  [[nodiscard]] char *end() const {
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

  static constexpr FixedString min() {
    FixedString ret;
    memset(ret.key, 0, sizeof(ret.key));
    return ret;
  }

  static constexpr FixedString max() {
    FixedString ret;
    memset(ret.key, 0xFF, sizeof(ret.key));
    return ret;
  }

  [[nodiscard]] bool empty() const {
    return key[0] == '\0';
  }
};

struct Node {
  FixedString<64> name;
  long long value;

  Node() = default;

  Node(const FixedString<64> &name, long long value) : name(name), value(value) {}

  bool operator<(const Node &rhs) const {
    return name < rhs.name || (name == rhs.name && value < rhs.value);
  }

  bool operator==(const Node &rhs) const {
    return name == rhs.name && value == rhs.value;
  }
};

int N;
PersistentSet<Node, 4, 4> test2("root");

int main() {
  freopen("in.txt", "r", stdin);
  freopen("out2.txt", "w", stdout);
  long long value;
  std::cin >> N;
  std::string s;
  for (int i = 1; i <= N; i++) {
    std::cin >> s;
    if (s[0] == 'i') {
      std::cin >> s;
      std::cin >> value;
      test2.insert(Node(s, value));
    } else if (s[0] == 'f') {
      std::cin >> s;
      bool flag = false;
      auto iterator1 = test2.find(Node(s, INT64_MIN));
      auto iterator2 = test2.find(Node(s, INT64_MAX));
      for (auto n = iterator1; n != iterator2; n++) {
        flag = true;
        std::cout << (*n).value << " ";
      }
      std::cout << (flag ? "\n" : "null\n");
    } else {
      std::cin >> s;
      std::cin >> value;
      test2.erase(Node(s, value));
    }
  }
  return 0;
}
