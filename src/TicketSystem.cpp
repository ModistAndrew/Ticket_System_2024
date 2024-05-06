#include "PersistentSet.hpp"
int N, V;
std::string S;

PersistentSet<pair<FixedString<64>, int>, 120, 120, 8000> set("set");
int main() {
//  freopen("input.txt", "r", stdin);
//  freopen("output.txt", "w", stdout);
//  auto start = std::chrono::high_resolution_clock::now();
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
//  auto end = std::chrono::high_resolution_clock::now();
//  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
//  std::cerr << "Total program run time: " << duration.count() << " milliseconds" << std::endl;
  return 0;
}