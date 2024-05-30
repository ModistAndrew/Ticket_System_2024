//
// Created by zjx on 2024/5/26.
//

#ifndef TICKETSYSTEM2024_TRAIN_HPP
#define TICKETSYSTEM2024_TRAIN_HPP

#include <utility>

#include "PersistentMap.hpp"
#include "SimpleFile.hpp"
#include "PersistentSet.hpp"
#include "Util.hpp"
#include "priority_queue.hpp"
#include "list.hpp"

struct Train {
  String20 trainID; //train ID
  int trainData; //where train data is stored
  using INDEX = String20;

  INDEX index() const {
    return trainID;
  }
};

struct TrainInfo {
  std::string *base = nullptr; //may be nullptr when first storing
  std::string trainID;
  int stationNum;
  int seatNum;
  vector<std::string> stationNames; //stationName[0] is the start station. size == stationNum
  vector<int> prices; //prices from start station to station[i]. size == stationNum. 0 for start station
  vector<int> arrivalTimes; //time from start time to arrival at station[i]. size == stationNum. invalid for start station
  vector<int> departureTimes; //time from start time to departure from station[i]. size == stationNum. invalid for end station. start time for start station
  int firstStartDate;
  int totalCount; //total number of trains. from startDate to startDate + totalDate - 1
  std::string type; //type of the train
  vector<int> seats; //seats[i * (stationNum - 1) + j] is the number of seats of station[j] on the ith train. size == train_num * (stationNum - 1)

  TrainInfo() = default;

  TrainInfo(std::string trainID, int stationNum, int seatNum, int firstStartDate, int totalCount, std::string type) :
    trainID(std::move(trainID)), stationNum(stationNum), seatNum(seatNum), firstStartDate(firstStartDate),
    totalCount(totalCount), type(std::move(type)),
    stationNames(stationNum), prices(stationNum), arrivalTimes(stationNum), departureTimes(stationNum),
    seats(totalCount * (stationNum - 1)) {
    for (int i = 0; i < totalCount * (stationNum - 1); i++) {
      seats[i] = seatNum;
    }
  }

  explicit TrainInfo(std::string *s) {
    base = s;
    vector<std::string> v = parseVector(*s, ' ', 11);
    trainID = v[0];
    stationNum = parseInt(v[1]);
    seatNum = parseInt(v[2]);
    stationNames = parseVector(v[3], '|', stationNum);
    prices = parseIntVector(v[4], '|', stationNum);
    arrivalTimes = parseIntVector(v[5], '|', stationNum);
    departureTimes = parseIntVector(v[6], '|', stationNum);
    firstStartDate = parseInt(v[7]);
    totalCount = parseInt(v[8]);
    type = v[9];
    seats = parseFixedIntVector(6, v[10], totalCount * (stationNum - 1));
  }

  std::string toString() const {
    vector<std::string> v(11);
    v[0] = trainID;
    v[1] = toStringInt(stationNum);
    v[2] = toStringInt(seatNum);
    v[3] = toStringVector(stationNames, '|', stationNum);
    v[4] = toStringIntVector(prices, '|', stationNum);
    v[5] = toStringIntVector(arrivalTimes, '|', stationNum);
    v[6] = toStringIntVector(departureTimes, '|', stationNum);
    v[7] = toStringInt(firstStartDate);
    v[8] = toStringInt(totalCount);
    v[9] = type;
    v[10] = toFixedStringIntVector(6, seats, totalCount * (stationNum - 1));
    return toStringVector(v, ' ', 11);
  }

  void store() {
    *base = toString();
  }

  int toTrainNum(int startDate) const {
    return startDate - firstStartDate;
  }

  //search the train num when the train depart from station[stationIndex] on departureDate
  //may < 0 or >= totalCount. you should check it
  int findTrainNum(int departureDate, int stationIndex) const {
    return departureDate - getDeparture(0, stationIndex).date;
  }

  //search the min train num when the train depart from station[stationIndex] after minDeparture
  //return -1 if no such train
  int findBestTrainNum(Chrono minDeparture, int stationIndex) const {
    int ret = findTrainNum(minDeparture.date, stationIndex);
    if (getDeparture(ret, stationIndex).toTick() < minDeparture.toTick()) {
      ret++;
    }
    if (ret < 0) {
      return 0;
    }
    if (ret >= totalCount) {
      return -1;
    }
    return ret;
  }

  int searchStationIndex(const std::string &stationName) const {
    for (int i = 0; i < stationNum; i++) {
      if (stationNames[i] == stationName) {
        return i;
      }
    }
    return -1;
  }

  Chrono getArrival(int trainNum, int stationIndex) const {
    return stationIndex == 0 ? Chrono() : Chrono(firstStartDate + trainNum, arrivalTimes[stationIndex]);
  }

  Chrono getDeparture(int trainNum, int stationIndex) const {
    return stationIndex == stationNum - 1 ? Chrono() : Chrono(firstStartDate + trainNum, departureTimes[stationIndex]);
  }

  int getSeat(int trainNum, int stationIndex) const { //stationIndex should be less than stationNum - 1
    return seats[trainNum * (stationNum - 1) + stationIndex];
  }

  int getMaxSeat(int trainNum, int startStationIndex, int endStationIndex) const {
    int res = seats[trainNum * (stationNum - 1) + startStationIndex];
    for (int i = startStationIndex + 1; i < endStationIndex; i++) {
      res = std::min(res, seats[trainNum * (stationNum - 1) + i]);
    }
    return res;
  }

  int getPrice(int startStationIndex, int endStationIndex) const {
    return prices[endStationIndex] - prices[startStationIndex];
  }

  int buy(int trainNum, int startStationIndex, int endStationIndex, int num) {
    for (int i = startStationIndex; i < endStationIndex; i++) {
      if (seats[trainNum * (stationNum - 1) + i] < num) {
        return -1;
      }
    }
    for (int i = startStationIndex; i < endStationIndex; i++) {
      seats[trainNum * (stationNum - 1) + i] -= num;
    }
    store();
    return num * getPrice(startStationIndex, endStationIndex);
  }

  void refund(int trainNum, int startStationIndex, int endStationIndex, int num) {
    for (int i = startStationIndex; i < endStationIndex; i++) {
      seats[trainNum * (stationNum - 1) + i] += num;
    }
    store();
  }
};

struct Station {
  String40 station; //station name
  int trainData; //where train data is stored
  int stationNum; //index of the station in the train
  auto operator<=>(const Station &rhs) const = default;
};

struct Line {
  String20 trainID;
  String40 from;
  Chrono departure;
  String40 to;
  Chrono arrival;
  int price;
  int seat;
  int time;

  static bool cmpTime(const Line &lhs, const Line &rhs) {
    return lhs.time > rhs.time || (lhs.time == rhs.time && lhs.trainID > rhs.trainID);
  }

  static bool cmpPrice(const Line &lhs, const Line &rhs) {
    return lhs.price > rhs.price || (lhs.price == rhs.price && lhs.trainID > rhs.trainID);
  }

  static bool cmpTimePair(const pair<Line, Line> &lhs, const pair<Line, Line> &rhs) {
    int totalTime1 = lhs.second.arrival.toTick() - lhs.first.departure.toTick();
    int totalTime2 = rhs.second.arrival.toTick() - rhs.first.departure.toTick();
    int totalPrice1 = lhs.first.price + lhs.second.price;
    int totalPrice2 = rhs.first.price + rhs.second.price;
    return totalTime1 > totalTime2 || (totalTime1 == totalTime2 && totalPrice1 > totalPrice2) ||
           (totalTime1 == totalTime2 && totalPrice1 == totalPrice2 && lhs.first.trainID > rhs.first.trainID) ||
           (totalTime1 == totalTime2 && totalPrice1 == totalPrice2 && lhs.first.trainID == rhs.first.trainID &&
            lhs.second.trainID > rhs.second.trainID);
  }

  static bool cmpPricePair(const pair<Line, Line> &lhs, const pair<Line, Line> &rhs) {
    int totalPrice1 = lhs.first.price + lhs.second.price;
    int totalPrice2 = rhs.first.price + rhs.second.price;
    int totalTime1 = lhs.second.arrival.toTick() - lhs.first.departure.toTick();
    int totalTime2 = rhs.second.arrival.toTick() - rhs.first.departure.toTick();
    return totalPrice1 > totalPrice2 || (totalPrice1 == totalPrice2 && totalTime1 > totalTime2) ||
           (totalPrice1 == totalPrice2 && totalTime1 == totalTime2 && lhs.first.trainID > rhs.first.trainID) ||
           (totalPrice1 == totalPrice2 && totalTime1 == totalTime2 && lhs.first.trainID == rhs.first.trainID &&
            lhs.second.trainID > rhs.second.trainID);
  }

  friend std::ostream &operator<<(std::ostream &out, const Line &rhs) {
    out << rhs.trainID << ' ' << rhs.from << ' ' << rhs.departure << " -> " << rhs.to << ' ' << rhs.arrival << ' '
        << rhs.price << ' ' << rhs.seat;
    return out;
  }
};

namespace Trains {
  PersistentMap<Train> unreleasedTrainMap("unreleased_train");
  PersistentMap<Train> releasedTrainMap("released_train");
  PersistentSet<Station> stationMap("station");
  SimpleFile trainDataFile("train_data");

  bool addTrain(const TrainInfo &trainInfo) {
    String20 index = trainInfo.trainID;
    std::string data = trainInfo.toString();
    if (unreleasedTrainMap.get(index).second || releasedTrainMap.get(index).second) {
      return false;
    }
    Train train{index, trainDataFile.write(data)};
    unreleasedTrainMap.insert(train);
    return true;
  }

  bool deleteTrain(const String20 &index) {
    return unreleasedTrainMap.erase(index);
  }

  bool releaseTrain(const String20 &index) {
    auto it = unreleasedTrainMap.get(index);
    if (!it.second) {
      return false;
    }
    Train train = *it.first;
    unreleasedTrainMap.erase(index);
    if (!releasedTrainMap.insert(train)) {
      throw;
    }
    TrainInfo trainInfo = TrainInfo(trainDataFile.get(train.trainData, false));
    for (int i = 0; i < trainInfo.stationNum; i++) {
      stationMap.insert(Station{trainInfo.stationNames[i], train.trainData, i});
    }
    return true;
  }

  Optional<TrainInfo> getTrain(const String20 &index, bool dirty, bool shouldRelease) {
    if (!shouldRelease) {
      auto it1 = unreleasedTrainMap.get(index);
      if (it1.second) {
        return {TrainInfo(trainDataFile.get(it1.first->trainData, dirty))};
      }
    }
    auto it2 = releasedTrainMap.get(index);
    if (it2.second) {
      return {TrainInfo(trainDataFile.get(it2.first->trainData, dirty))};
    }
    return {};
  }

  void queryTicket(const String40 &from, const String40 &to, int date, bool isPrice) {
    auto it1 = stationMap.find({from, 0, 0});
    auto it2 = stationMap.find({to, 0, 0});
    priority_queue<Line> queue(isPrice ? Line::cmpPrice : Line::cmpTime);
    while (!it1.end() && it1->station == from && !it2.end() && it2->station == to) {
      if (it1->trainData == it2->trainData && it1->stationNum < it2->stationNum) {
        TrainInfo trainInfo = TrainInfo(trainDataFile.get(it1->trainData, false));
        int trainNum = trainInfo.findTrainNum(date, it1->stationNum);
        if (trainNum >= 0 && trainNum < trainInfo.totalCount) {
          Chrono departure = trainInfo.getDeparture(trainNum, it1->stationNum);
          Chrono arrival = trainInfo.getArrival(trainNum, it2->stationNum);
          int seat = trainInfo.getMaxSeat(trainNum, it1->stationNum, it2->stationNum);
          queue.push(Line{trainInfo.trainID,
                          from, departure,
                          to, arrival,
                          trainInfo.getPrice(it1->stationNum, it2->stationNum),
                          seat,
                          arrival.toTick() - departure.toTick()});
        }
      }
      if (it1->trainData < it2->trainData) {
        it1++;
      } else {
        it2++;
      }
    }
    std::cout << queue.size();
    while (!queue.empty()) {
      std::cout << '\n' << queue.top();
      queue.pop();
    }
  }

  bool queryTransfer(const String40 &from, const String40 &to, int date, bool isPrice) {
    auto it1 = stationMap.find({from, 0, 0});
    auto it2 = stationMap.find({to, 0, 0});
    list<TrainInfo> trainsFrom;
    list<TrainInfo> trainsTo;
    while (!it1.end() && it1->station == from) {
      trainsFrom.push_back(TrainInfo(trainDataFile.get(it1->trainData, false)));
      it1++;
    }
    while (!it2.end() && it2->station == to) {
      trainsTo.push_back(TrainInfo(trainDataFile.get(it2->trainData, false)));
      it2++;
    }
    priority_queue<pair<Line, Line>> queue(isPrice ? Line::cmpPricePair : Line::cmpTimePair);
    for (const auto &trainFrom: trainsFrom) {
      for (const auto &trainTo: trainsTo) {
        map<String40, int> stationIndicesFrom, stationIndicesTo;
        for (int i = 0; i < trainFrom.stationNum; i++) {
          stationIndicesFrom[trainFrom.stationNames[i]] = i;
        }
        for (int i = 0; i < trainTo.stationNum; i++) {
          stationIndicesTo[trainTo.stationNames[i]] = i;
        }
        int indexFrom = stationIndicesFrom[from];
        int indexTo = stationIndicesTo[to];
        for (int intersectionIndexFrom = indexFrom + 1; intersectionIndexFrom < trainFrom.stationNum; intersectionIndexFrom++) {
          String40 intersection = trainFrom.stationNames[intersectionIndexFrom];
          auto it = stationIndicesTo.find(intersection);
          if (it != stationIndicesTo.end()) {
            int intersectionIndexTo = it->second;
            if (intersectionIndexTo < indexTo) {
              int trainNumFrom = trainFrom.findTrainNum(date, indexFrom);
              if (trainNumFrom < 0 || trainNumFrom >= trainFrom.totalCount) {
                continue;
              }
              Chrono departureFrom = trainFrom.getDeparture(trainNumFrom, indexFrom);
              Chrono arrivalFrom = trainFrom.getArrival(trainNumFrom, intersectionIndexFrom);
              int trainNumTo = trainTo.findBestTrainNum(arrivalFrom, intersectionIndexTo);
              if (trainNumTo < 0) {
                continue;
              }
              Chrono departureTo = trainTo.getDeparture(trainNumTo, intersectionIndexTo);
              Chrono arrivalTo = trainTo.getArrival(trainNumTo, indexTo);
              queue.push({Line{trainFrom.trainID,
                               from, departureFrom,
                               intersection, arrivalFrom,
                               trainFrom.getPrice(indexFrom, intersectionIndexFrom),
                               trainFrom.getMaxSeat(trainNumFrom, indexFrom, intersectionIndexFrom),
                               arrivalFrom.toTick() - departureFrom.toTick()},
                          Line{trainTo.trainID,
                               intersection, departureTo,
                               to, arrivalTo,
                               trainTo.getPrice(intersectionIndexTo, indexTo),
                               trainTo.getMaxSeat(trainNumTo, intersectionIndexTo, indexTo),
                               arrivalTo.toTick() - departureTo.toTick()}});
            }
          }
        }
      }
    }
    if (queue.empty()) {
      return false;
    }
    std::cout << queue.top().first << '\n' << queue.top().second;
    return true;
  }
}

#endif