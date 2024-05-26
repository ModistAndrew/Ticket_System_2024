//
// Created by zjx on 2024/5/26.
//

#ifndef TICKETSYSTEM2024_TRAIN_HPP
#define TICKETSYSTEM2024_TRAIN_HPP

#include "PersistentMap.hpp"
#include "SimpleFile.hpp"
#include "Util.hpp"

struct Train {
  String20 index;
  int location; //where train data is stored
};

struct TrainInfo {
  std::string trainID;
  int stationNum;
  vector<std::string> stationNames; //stationName[0] is the start station. size == stationNum
  int seatNum;
  vector<int> prices; //prices[0] is the price from station[0] to station[1]. size == stationNum - 1
  int startTime; //start time in minutes, 0 - 1439
  vector<int> travelTimes; //travelTimes[0] is the time from station[0] to station[1]. in minutes. size == stationNum - 1
  vector<int> stopoverTimes; //stopoverTimes[0] is the stopover time at station[1]. in minutes. size == stationNum - 2
  vector<int> saleDate; //start and end day from 20240601 to 20240831. converted to an integer 0 - 91. size == 2
  std::string type; //type of the train

  TrainInfo() = default;

  TrainInfo(const std::string& s) {
    vector<std::string> v = parseVector(s, ' ');
    trainID = v[0];
    stationNum = parseInt(v[1]);
    stationNames = parseVector(v[2], '|');
    seatNum = parseInt(v[3]);
    prices = parseIntVector(v[4], '|');
    startTime = parseInt(v[5]);
    travelTimes = parseIntVector(v[6], '|');
    stopoverTimes = parseIntVector(v[7], '|');
    saleDate = parseIntVector(v[8], '|');
    type = v[9];
  }

  std::string toString() const {
    vector<std::string> v;
    v.push_back(trainID);
    v.push_back(toStringInt(stationNum));
    v.push_back(toStringVector(stationNames, '|'));
    v.push_back(toStringInt(seatNum));
    v.push_back(toStringIntVector(prices, '|'));
    v.push_back(toStringInt(startTime));
    v.push_back(toStringIntVector(travelTimes, '|'));
    v.push_back(toStringIntVector(stopoverTimes, '|'));
    v.push_back(toStringIntVector(saleDate, '|'));
    v.push_back(type);
    return toStringVector(v, ' ');
  }
};

namespace Trains {
  PersistentMap<Train> unreleasedTrainMap("unreleased_train");
  PersistentMap<Train> releasedTrainMap("released_train");
  SimpleFile trainDataFile("train_data");

  bool addTrain(const TrainInfo &trainInfo) {
    String20 index = trainInfo.trainID;
    std::string data = trainInfo.toString();
    if(unreleasedTrainMap.find(index).second || releasedTrainMap.find(index).second) {
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
    if(!it.second) {
      return false;
    }
    Train train = *it.first;
    unreleasedTrainMap.erase(index);
    if(!releasedTrainMap.insert(train)) {
      throw;
    }
    return true;
  }

  Optional<TrainInfo> getTrain(const String20 &index) {
    auto it1 = unreleasedTrainMap.find(index);
    if(it1.second) {
      return {trainDataFile.read(it1.first->location)};
    }
    auto it2 = releasedTrainMap.find(index);
    if(it2.second) {
      return {trainDataFile.read(it2.first->location)};
    }
    return {};
  }
}

#endif //TICKETSYSTEM2024_TRAIN_HPP