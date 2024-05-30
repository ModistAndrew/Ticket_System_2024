#include "Command.hpp"
#include "SimpleFile.hpp"
#include "PersistentMultiMap.hpp"

int main() {
//  freopen("testcases/pressure_1_easy/44.in", "r", stdin);
//  freopen("test.out", "w", stdout);
  Commands::init();
  while (Commands::running) {
    std::string input;
    getline(std::cin, input);
    std::cout << Commands::run(input) << '\n';
  }
  return 0;
}
