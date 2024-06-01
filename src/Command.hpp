//
// Created by zjx on 2024/5/26.
//

#ifndef TICKETSYSTEM2024_COMMAND_HPP
#define TICKETSYSTEM2024_COMMAND_HPP

#include "Account.hpp"
#include "Train.hpp"
#include "Order.hpp"

struct Command {
  std::string timestamp;
  std::string name;
  std::string params[26];

  explicit Command(const std::string &s) {
    int status = 0;
    char p;
    for (char c: s) {
      if (c == ' ') {
        status++;
        continue;
      }
      if (status == 0) {
        timestamp.push_back(c);
      } else if (status == 1) {
        name.push_back(c);
      } else if (status % 2 == 0) {
        if (c != '-') {
          p = c;
        }
      } else {
        params[p - 'a'].push_back(c);
      }
    }
  }

  const std::string &getParam(char c) const {
    return params[c - 'a'];
  }

  int getIntParam(char c) const {
    return parseInt(params[c - 'a']);
  }
};

namespace Commands {
  typedef std::string (*CommandFunc)(const Command &);

  map<std::string, CommandFunc> commandMap;
  bool running = true;

  std::string addUser(const Command &command) {
    Account newAccount(command.getParam('u'), command.getParam('p'), command.getParam('n'), command.getParam('m'),
                       command.getIntParam('g'));
    if (AccountStorage::empty()) {
      newAccount.privilege = 10;
      AccountStorage::add(newAccount);
      return "0";
    } else {
      auto currentAccount = Accounts::getLogged(command.getParam('c'));
      if (!currentAccount.present || currentAccount.value->privilege <= newAccount.privilege) {
        return "-1";
      }
      return AccountStorage::add(newAccount) ? "0" : "-1";
    }
  }

  std::string login(const Command &command) {
    return Accounts::login(command.getParam('u'), command.getParam('p')) ? "0" : "-1";
  }

  std::string logout(const Command &command) {
    return Accounts::logout(command.getParam('u')) ? "0" : "-1";
  }

  std::string queryProfile(const Command &command) {
    auto currentAccount = Accounts::getLogged(command.getParam('c'));
    auto queryAccount = AccountStorage::get(command.getParam('u'), false);
    if (!currentAccount.present || !queryAccount.present ||
        (currentAccount.value->userID != queryAccount.value->userID &&
         currentAccount.value->privilege <= queryAccount.value->privilege)) {
      return "-1";
    }
    std::cout << *queryAccount.value;
    return "";
  }

  std::string modifyProfile(const Command &command) {
    auto currentAccount = Accounts::getLogged(command.getParam('c'));
    auto modifyAccount = AccountStorage::get(command.getParam('u'), true);
    if (!currentAccount.present || !modifyAccount.present ||
        (currentAccount.value->userID != modifyAccount.value->userID &&
         currentAccount.value->privilege <= modifyAccount.value->privilege)) {
      return "-1";
    }
    Account* newAccount = modifyAccount.value;
    if (!command.getParam('g').empty()) {
      if (command.getIntParam('g') >= currentAccount.value->privilege) {
        return "-1";
      }
      newAccount->privilege = command.getIntParam('g');
    }
    if (!command.getParam('p').empty()) {
      newAccount->password = command.getParam('p');
    }
    if (!command.getParam('n').empty()) {
      newAccount->name = command.getParam('n');
    }
    if (!command.getParam('m').empty()) {
      newAccount->mailAddr = command.getParam('m');
    }
    std::cout << *newAccount;
    return "";
  }

  std::string addTrain(const Command &command) {
    int stationNum = command.getIntParam('n');
    vector<string> v = parseVector(command.getParam('d'), '|', 2);
    int startDate = parseDate(v[0]);
    int endDate = parseDate(v[1]);
    TrainInfo newTrain(command.getParam('i'), stationNum, command.getIntParam('m'),
                       startDate, endDate - startDate + 1, command.getParam('y'));
    newTrain.stationNames = parseVector(command.getParam('s'), '|', stationNum);
    vector<int> prices = parseIntVector(command.getParam('p'), '|', stationNum - 1);
    int currentPrice = 0;
    for (int i = 0; i < stationNum; i++) {
      newTrain.prices[i] = currentPrice;
      if (i < stationNum - 1) {
        currentPrice += prices[i];
      }
    }
    vector<int> travelTimes = parseIntVector(command.getParam('t'), '|', stationNum - 1);
    vector<int> stopoverTimes = parseIntVector(command.getParam('o'), '|', stationNum - 2);
    int currentTime = parseTime(command.getParam('x'));
    newTrain.departureTimes[0] = currentTime;
    for (int i = 1; i < stationNum; i++) {
      currentTime += travelTimes[i - 1];
      newTrain.arrivalTimes[i] = currentTime;
      if (i < stationNum - 1) {
        currentTime += stopoverTimes[i - 1];
        newTrain.departureTimes[i] = currentTime;
      }
    }
    return Trains::addTrain(newTrain) ? "0" : "-1";
  }

  std::string deleteTrain(const Command &command) {
    return Trains::deleteTrain(command.getParam('i')) ? "0" : "-1";
  }

  std::string releaseTrain(const Command &command) {
    return Trains::releaseTrain(command.getParam('i')) ? "0" : "-1";
  }

  std::string queryTrain(const Command &command) {
    bool isReleased;
    auto train = Trains::getTrain(command.getParam('i'), false, false);
    if (!train.present) {
      return "-1";
    }
    TrainInfo &trainInfo = *train.value;
    int trainNum = trainInfo.toTrainNum(parseDate(command.getParam('d')));
    if (trainNum < 0 || trainNum >= trainInfo.totalCount) {
      return "-1";
    }
    std::cout << trainInfo.trainID << ' ' << trainInfo.type << '\n';
    for (int i = 0; i < trainInfo.stationNum; i++) {
      std::cout << trainInfo.stationNames[i] << ' '
                << trainInfo.getArrival(trainNum, i) << " -> "
                << trainInfo.getDeparture(trainNum, i) << ' '
                << trainInfo.prices[i] << ' ';
      if (i < trainInfo.stationNum - 1) {
        std::cout << trainInfo.getSeat(trainNum, i) << '\n';
      } else {
        std::cout << "x";
      }
    }
    return "";
  }

  std::string buyTicket(const Command &command) {
    String20 userID = command.getParam('u');
    String20 trainID = command.getParam('i');
    auto train = Trains::getTrain(trainID, true, true);
    if (!train.present) {
      return "-1";
    }
    auto user = Accounts::getLogged(userID);
    if (!user.present) {
      return "-1";
    }
    TrainInfo &trainInfo = *train.value;
    int startStation = trainInfo.searchStationIndex(command.getParam('f'));
    int endStation = trainInfo.searchStationIndex(command.getParam('t'));
    if (startStation < 0 || endStation < 0 || startStation >= endStation) {
      return "-1";
    }
    int trainNum = trainInfo.findTrainNum(parseDate(command.getParam('d')), startStation);
    if (trainNum < 0 || trainNum >= trainInfo.totalCount) {
      return "-1";
    }
    int count = command.getIntParam('n');
    if(count > trainInfo.seatNum) {
      return "-1";
    }
    bool shouldQueue = command.getParam('q') == "true";
    int price = trainInfo.buy(trainNum, startStation, endStation, count);
    Order order = {
      userID,
      trainID,
      trainNum,
      price < 0 ? 1 : 0,
      startStation,
      command.getParam('f'),
      trainInfo.getDeparture(trainNum, startStation),
      endStation,
      command.getParam('t'),
      trainInfo.getArrival(trainNum, endStation),
      trainInfo.getPrice(startStation, endStation),
      count
    };
    if (price < 0) {
      if (shouldQueue) {
        Orders::addOrder(order);
        return "queue";
      }
      return "-1";
    }
    Orders::addOrder(order);
    std::cout << price;
    return "";
  }

  std::string queryTicket(const Command &command) {
    Trains::queryTicket(command.getParam('s'), command.getParam('t'), parseDate(command.getParam('d')),
                        command.getParam('p') == "cost");
    return "";
  }

  std::string queryOrder(const Command &command) {
    auto user = Accounts::getLogged(command.getParam('u'));
    if (!user.present) {
      return "-1";
    }
    Orders::printOrders(user.value->userID);
    return "";
  }

  std::string refundTicket(const Command &command) {
    auto user = Accounts::getLogged(command.getParam('u'));
    if (!user.present) {
      return "-1";
    }
    return Orders::refundOrder(user.value->userID, command.getParam('n').empty() ? 1 : command.getIntParam('n')) ? "0" : "-1";
  }

  std::string queryTransfer(const Command &command) {
    return Trains::queryTransfer(command.getParam('s'), command.getParam('t'), parseDate(command.getParam('d')),
                                 command.getParam('p') == "cost") ? "" : "0";
  }

  std::string clean(const Command &command) {
    throw;
    return "0";
  }

  std::string exit(const Command &command) {
    running = false;
    return "bye";
  }

  void init() {
    commandMap["add_user"] = addUser;
    commandMap["login"] = login;
    commandMap["logout"] = logout;
    commandMap["query_profile"] = queryProfile;
    commandMap["modify_profile"] = modifyProfile;
    commandMap["exit"] = exit;
    commandMap["add_train"] = addTrain;
    commandMap["delete_train"] = deleteTrain;
    commandMap["release_train"] = releaseTrain;
    commandMap["query_train"] = queryTrain;
    commandMap["buy_ticket"] = buyTicket;
    commandMap["query_order"] = queryOrder;
    commandMap["query_ticket"] = queryTicket;
    commandMap["refund_ticket"] = refundTicket;
    commandMap["query_transfer"] = queryTransfer;
    commandMap["clean"] = clean;
  }

  std::string run(const std::string &s) {
    Command command(s);
    auto it = commandMap.find(command.name);
    std::cout << command.timestamp << ' ';
    if (it == commandMap.end()) {
      return "-1";
    }
    return it->second(command);
  }
}

#endif
