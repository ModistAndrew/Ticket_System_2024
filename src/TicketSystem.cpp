#include "PersistentSet.hpp"
int N, V;
std::string S;

PersistentSet<pair<FixedString<64>, int>, 400, 400, 200> set("set");
int main() {
  std::cin >> N;
  for (int i = 1; i <= N; i++) {
    std::cin >> S;
    if (S[0] == 'i') {
      std::cin >> S;
      std::cin >> V;
      set.insert(pair(FixedString<64>(S), V));
    } else if (S[0] == 'f') {
      std::cin >> S;
      bool flag = false;
      auto iterator1 = set.lowerBound(pair(FixedString<64>(S), INT32_MIN));
      auto iterator2 = set.upperBound(pair(FixedString<64>(S), INT32_MAX));
      for (auto n = iterator1; n != iterator2; n++) {
        flag = true;
        std::cout << (*n).second << " ";
      }
      std::cout << (flag ? "\n" : "null\n");
    } else {
      std::cin >> S;
      std::cin >> V;
      set.erase(pair(FixedString<64>(S), V));
    }
    set.checkCache();
  }
  return 0;
}