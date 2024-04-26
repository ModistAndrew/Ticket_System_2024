//
// Created by zjx on 2024/4/26.
//
#include "PersistentSet.hpp"

int main() {
  PersistentSet<int, 4, 4> set;
  int n = 1000;
  std::vector<int> v;
  for (int i = 0; i < n; i++) {
    v.push_back(i * i);
  }
  std::random_shuffle(v.begin(), v.end());
  for (int i = 0; i < n; i++) {
    std::cout << v[i] << ' ';
  }
  std::cout << '\n';
  for (int i: v) {
    std::cout << set.insert(i) << ' ';
  }
  std::cout << '\n';
  for (int i = 0; i < n / 2; i++) {
    std::cout << set.erase(i) << ' ';
  }
  std::cout << '\n';
  auto it = set.find(100);
  auto it1 = set.find(2000);
  while (it != it1) {
    std::cout << *it << ' ';
    it++;
  }
  return 0;
}
