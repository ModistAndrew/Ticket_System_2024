#include "Command.hpp"
#include "SimpleFile.hpp"
#include "PersistentMultiMap.hpp"

struct Data {
  int index;
  int val;
};

int main() {
  Commands::init();
  while (Commands::running) {
    std::string input;
    getline(std::cin, input);
    std::cout << Commands::run(input) << '\n';
  }
}
