#include "Command.hpp"
#include "SimpleFile.hpp"
#include "Account.hpp"
#include "Order.hpp"
#include "Train.hpp"

void checkCache() {
  AccountStorage::accountMap.checkCache();
  Orders::orderMap.checkCache();
  Orders::orderQueueMap.checkCache();
  Trains::unreleasedTrainMap.checkCache();
  Trains::releasedTrainMap.checkCache();
  Trains::stationMap.checkCache();
}

int main() {
//  freopen("testcases/pressure_1_easy/44.in", "r", stdin);
//  freopen("test.out", "w", stdout);
  Commands::init();
  while (Commands::running) {
    std::string input;
    getline(std::cin, input);
    std::cout << Commands::run(input) << '\n';
    checkCache();
  }
  return 0;
}
