#include "Command.hpp"
#include "SimpleFile.hpp"
#include "PersistentMultiMap.hpp"

struct Data {
  int id;
  String20 name;
  using INDEX = int;

  const INDEX &index() const {
    return id;
  }
};

int main() {
//  freopen("testcases/basic_4/12.in", "r", stdin);
//  freopen("test.out", "w", stdout);
//  freopen("test.in", "r", stdin);
  Commands::init();
  while (Commands::running) {
    std::string input;
    getline(std::cin, input);
    std::cout << Commands::run(input) << '\n';
  }
  return 0;
}
