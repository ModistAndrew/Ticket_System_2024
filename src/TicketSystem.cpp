#include "PersistentMap.hpp"
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

struct Data {
  pair<ull, int> index;
};

PersistentMap<Data, 800, 800, 4000> set("set");
int main() {
  std::cin >> N;
  for (int i = 1; i <= N; i++) {
    std::cin >> S;
    std::cin >> S0;
    ull h = hash(S0);
    if (S[0] == 'i') {
      std::cin >> V;
      set.insert({pair(h, V)});
    } else if (S[0] == 'f') {
      bool flag = false;
      auto iterator1 = set.find({pair(h, INT32_MIN)}).first;
      auto it2 = set.find({pair(h, INT32_MAX)});
      auto iterator2 = it2.first;
      if(it2.second) {
        iterator2++;
      }
      for (auto n = iterator1; n != iterator2; n++) {
        flag = true;
        std::cout << (*n).index.second << " ";
      }
      std::cout << (flag ? "\n" : "null\n");
    } else {
      std::cin >> V;
      set.erase(pair(h, V));
    }
  }
  return 0;
}