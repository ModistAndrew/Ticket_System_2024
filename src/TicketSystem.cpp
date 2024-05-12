#include "PersistentSet.hpp"
typedef unsigned long long ull;
int N, V;
std::string S, S0;

ull hash(const std::string &s) {
  ull h = 0;
  for (int i = 0; i < s.length(); i++) {
    h = h * 131 + s[i];
  }
  return h;
}

PersistentSet<pair<ull, int>, 800, 800, 4000> set("set");
int main() {
  std::cin >> N;
  for (int i = 1; i <= N; i++) {
    std::cin >> S;
    std::cin >> S0;
    ull h = hash(S0);
    if (S[0] == 'i') {
      std::cin >> V;
      set.insert(pair(h, V));
    } else if (S[0] == 'f') {
      bool flag = false;
      auto iterator1 = set.lowerBound(pair(h, INT32_MIN));
      auto iterator2 = set.upperBound(pair(h, INT32_MAX));
      for (auto n = iterator1; n != iterator2; n++) {
        flag = true;
        std::cout << (*n).second << " ";
      }
      std::cout << (flag ? "\n" : "null\n");
    } else {
      std::cin >> V;
      set.erase(pair(h, V));
    }
  }
  return 0;
}