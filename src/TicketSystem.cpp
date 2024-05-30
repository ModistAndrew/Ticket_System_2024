#include "Command.hpp"
#include "SimpleFile.hpp"
#include "PersistentMultiMap.hpp"

int main() {
  freopen("testcases/basic_4/12.in", "r", stdin);
  freopen("test.out", "w", stdout);
//  freopen("test.in", "r", stdin);
  Commands::init();
  while (Commands::running) {
    std::string input;
    getline(std::cin, input);
    std::cout << Commands::run(input) << '\n';
  }
}
