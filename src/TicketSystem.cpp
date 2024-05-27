#include "Command.hpp"
#include "SimpleFile.hpp"
#include "PersistentMultiMap.hpp"

struct Data {
  int index;
  int val;
};

int main() {
//  Commands::init();
//  while (Commands::running) {
//    std::string input;
//    getline(std::cin, input);
//    std::cout << Commands::run(input) << '\n';
//  }
  PersistentMultiMap<Data> test("test");

  for (int i = 0; i < 100; i++) {
    auto it = test.find(i);
    std::cout << i << '\n';
    while (!it.end() && it->index == i) {
      std::cout << it->val << ' ';
      it->val = 1919;
      it.markDirty();
      ++it;
    }
    puts("");
  }
}
