//
// Created by zjx on 2024/5/26.
//

#ifndef TICKETSYSTEM2024_COMMAND_HPP
#define TICKETSYSTEM2024_COMMAND_HPP

#include "map.hpp"
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
      if (!currentAccount.present || currentAccount.value.privilege <= newAccount.privilege) {
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
    auto queryAccount = AccountStorage::get(command.getParam('u'));
    if (!currentAccount.present || !queryAccount.present ||
        (currentAccount.value.index != queryAccount.value.index &&
         currentAccount.value.privilege <= queryAccount.value.privilege)) {
      return "-1";
    }
    std::cout << queryAccount.value;
    return "";
  }

  std::string modifyProfile(const Command &command) {
    auto currentAccount = Accounts::getLogged(command.getParam('c'));
    auto modifyAccount = AccountStorage::get(command.getParam('u'));
    if (!currentAccount.present || !modifyAccount.present ||
        (currentAccount.value.index != modifyAccount.value.index &&
         currentAccount.value.privilege <= modifyAccount.value.privilege)) {
      return "-1";
    }
    Account newAccount = modifyAccount.value;
    if (!command.getParam('p').empty()) {
      newAccount.password = command.getParam('p');
    }
    if (!command.getParam('n').empty()) {
      newAccount.name = command.getParam('n');
    }
    if (!command.getParam('m').empty()) {
      newAccount.mailAddr = command.getParam('m');
    }
    if (!command.getParam('g').empty()) {
      if (command.getIntParam('g') >= currentAccount.value.privilege) {
        return "-1";
      }
      newAccount.privilege = command.getIntParam('g');
    }
    std::cout << newAccount;
    AccountStorage::modify(newAccount);
    return "";
  }

  std::string addTrain(const Command &command) {
    TrainInfo newTrain;
    newTrain.trainID = command.getParam('i');
    newTrain.stationNum = command.getIntParam('n');
    newTrain.seatNum = command.getIntParam('m');
    newTrain.stationNames = parseVector(command.getParam('s'), '|');
    newTrain.prices = parseIntVector(command.getParam('p'), '|');
    newTrain.startTime = parseTime(command.getParam('x'));
    newTrain.arrivalTimes = parseIntVector(command.getParam('t'), '|');
    newTrain.departureTimes = parseIntVector(command.getParam('o'), '|');
    vector<int> saleDate;
    vector<string> v = parseVector(command.getParam('d'), '|');
    for (const string &s: v) {
      saleDate.push_back(parseDate(s));
    }
    newTrain.saleDate = saleDate;
    newTrain.type = command.getParam('y');
    newTrain.initSeats();
    return Trains::addTrain(newTrain) ? "0" : "-1";
  }

  std::string deleteTrain(const Command &command) {
    return Trains::deleteTrain(command.getParam('i')) ? "0" : "-1";
  }

  std::string releaseTrain(const Command &command) {
    return Trains::releaseTrain(command.getParam('i')) ? "0" : "-1";
  }

  std::string queryTrain(const Command &command) {
    auto train = Trains::getTrain(command.getParam('i'), false, false);
    if (!train.present) {
      return "-1";
    }
    int date = parseDate(command.getParam('d'));
    TrainInfo &trainInfo = train.value;
    if (!trainInfo.checkDate(date)) {
      return "-1";
    }
    std::cout << trainInfo.trainID << ' ' << trainInfo.type << '\n';
    Chrono currentChrono = {date, trainInfo.startTime};
    int currentPrice = 0;
    int *seats = trainInfo.getSeats(date);
    for (int i = 0; i < trainInfo.stationNum; i++) {
      std::cout << trainInfo.stationNames[i] << ' '
                << (i == 0 ? Chrono::EMPTY : currentChrono.toString()) << " -> ";
      if (i > 0 && i < trainInfo.stationNum - 1) {
        currentChrono += trainInfo.departureTimes[i - 1];
      }
      std::cout << (i == trainInfo.stationNum - 1 ? Chrono::EMPTY : currentChrono.toString()) << ' '
                << currentPrice << ' '
                << (i == trainInfo.stationNum - 1 ? "x" : toStringInt(*seats));
      if (i < trainInfo.stationNum - 1) {
        currentChrono += trainInfo.arrivalTimes[i];
        currentPrice += trainInfo.prices[i];
        seats++;
        std::cout << '\n';
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
    Order order = {
        command.getParam('u'),
        command.getParam('i'),
        0,
        parseDate(command.getParam('d')),
        command.getIntParam('f'),
        command.getIntParam('t'),
        command.getIntParam('n')
    };
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
  }

  std::string run(const std::string &s) {
    Command command(s);
    auto it = commandMap.find(command.name);
    if (it == commandMap.end()) {
      throw;
    }
    std::cout << command.timestamp << ' ';
    return it->second(command);
  }
}

#endif //TICKETSYSTEM2024_COMMAND_HPP
