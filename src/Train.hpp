//
// Created by zjx on 2024/5/26.
//

#ifndef TICKETSYSTEM2024_TRAIN_HPP
#define TICKETSYSTEM2024_TRAIN_HPP

#include <utility>

#include "PersistentMap.hpp"
#include "SimpleFile.hpp"
#include "Util.hpp"

struct Train {
  String20 index; //train ID
  int location; //where train data is stored
};

struct TrainInfo {
  std::string *base; //may be nullptr when first storing
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

  TrainInfo(std::string *s) {
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

  int toTrainNum(int startDate) {
    return startDate - firstStartDate;
  }

  int searchTrainNum(int departureDate,
                     int stationIndex) { //search the train num when the train depart from station[stationIndex] on departureDate
    return departureDate - getDeparture(0, stationIndex).date;
  }

  int getStationIndex(const std::string &stationName) {
    for (int i = 0; i < stationNum; i++) {
      if (stationNames[i] == stationName) {
        return i;
      }
    }
    throw;
  }

  Chrono getArrival(int trainNum, int stationIndex) {
    return stationIndex == 0 ? Chrono() : Chrono(firstStartDate + trainNum, arrivalTimes[stationIndex]);
  }

  Chrono getDeparture(int trainNum, int stationIndex) {
    return stationIndex == stationNum - 1 ? Chrono() : Chrono(firstStartDate + trainNum, departureTimes[stationIndex]);
  }

  int getSeat(int trainNum, int stationIndex) { //stationIndex should be less than stationNum - 1
    return seats[trainNum * (stationNum - 1) + stationIndex];
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
    return num * (prices[endStationIndex] - prices[startStationIndex]);
  }
};

namespace Trains {
  PersistentMap<Train> unreleasedTrainMap("unreleased_train");
  PersistentMap<Train> releasedTrainMap("released_train");
  SimpleFile trainDataFile("train_data");

  bool addTrain(const TrainInfo &trainInfo) {
    String20 index = trainInfo.trainID;
    std::string data = trainInfo.toString();
    if (unreleasedTrainMap.find(index).second || releasedTrainMap.find(index).second) {
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
    auto it = unreleasedTrainMap.find(index);
    if (!it.second) {
      return false;
    }
    Train train = *it.first;
    unreleasedTrainMap.erase(index);
    if (!releasedTrainMap.insert(train)) {
      throw;
    }
    return true;
  }

  Optional<TrainInfo> getTrain(const String20 &index, bool dirty, bool shouldRelease) {
    if (!shouldRelease) {
      auto it1 = unreleasedTrainMap.find(index);
      if (it1.second) {
        return {TrainInfo(trainDataFile.get(it1.first->location, dirty))};
      }
    }
    auto it2 = releasedTrainMap.find(index);
    if (it2.second) {
      return {TrainInfo(trainDataFile.get(it2.first->location, dirty))};
    }
    return {};
  }
}

#endif //TICKETSYSTEM2024_TRAIN_HPP