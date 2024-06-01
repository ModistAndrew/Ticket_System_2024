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
  Trains::trainDataFile.checkCache();
  Trains::seatDataFile.checkCache();
}

int main() {
//  freopen("testcases/basic_4/8.in", "r", stdin);
//  freopen("test.out", "w", stdout);
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::cout.tie(nullptr);
  Commands::init();
  while (Commands::running) {
    std::string input;
    getline(std::cin, input);
    std::cout << Commands::run(input) << '\n';
    checkCache();
  }
  return 0;
}