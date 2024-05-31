//
// Created by zjx on 2024/5/26.
//

#ifndef TICKETSYSTEM2024_TRAIN_HPP
#define TICKETSYSTEM2024_TRAIN_HPP

#include "PersistentMap.hpp"
#include "SimpleFile.hpp"
#include "PersistentSet.hpp"
#include "Util.hpp"

struct Train {
  String20 trainID; //train ID
  int trainData; //where train data is stored
  using INDEX = String20;

  const INDEX &index() const {
    return trainID;
  }
};

struct Seats {
  vector<int> seats;

  Seats() = default;

  Seats(int length, int initSeat) : seats(length) {
    for (int i = 0; i < length; i++) {
      seats[i] = initSeat;
    }
  }

  explicit Seats(const std::string &s) : seats(parseFixedIntVector(6, s, s.length() / 6)) {}

  std::string toString() const {
    return toFixedStringIntVector(6, seats, seats.size());
  }

  int &operator[](int i) {
    return seats[i];
  }
};

struct Station {
  String40 station; //station name
  int trainData; //where train data is stored
  int stationNum; //index of the station in the train
  auto operator<=>(const Station &rhs) const = default;
};

namespace Trains {
  extern SimpleFile<Seats, 0> seatDataFile;
}

struct TrainInfo {
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
  vector<int> seatLoc; //seats[i] points to the start of the seat info of train i. size == totalCount

  TrainInfo() = default;

  TrainInfo(std::string trainID, int stationNum, int seatNum, int firstStartDate, int totalCount, std::string type) :
    trainID(std::move(trainID)), stationNum(stationNum), seatNum(seatNum), firstStartDate(firstStartDate),
    totalCount(totalCount), type(std::move(type)),
    stationNames(stationNum), prices(stationNum), arrivalTimes(stationNum), departureTimes(stationNum),
    seatLoc(totalCount) {
    for (int i = 0; i < totalCount; i++) {
      seatLoc[i] = Trains::seatDataFile.write(Seats(stationNum, seatNum));
    }
  }

  explicit TrainInfo(const std::string &s) {
    vector<std::string> v = parseVector(s, ' ', 11);
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
    seatLoc = parseIntVector(v[10], '|', totalCount);
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
    v[10] = toStringIntVector(seatLoc, '|', totalCount);
    return toStringVector(v, ' ', 11);
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

  Seats *getSeats(int trainNum, bool dirty) const {
    return Trains::seatDataFile.get(seatLoc[trainNum], dirty);
  }

  int getSeat(int trainNum, int stationIndex) const {
    return getSeats(trainNum, false)->operator[](stationIndex);
  }

  int getMaxSeat(int trainNum, int startStationIndex, int endStationIndex) const {
    Seats &seats = *getSeats(trainNum, false);
    int ret = seats[startStationIndex];
    for (int i = startStationIndex + 1; i < endStationIndex; i++) {
      ret = std::min(ret, seats[i]);
    }
    return ret;
  }

  int getPrice(int startStationIndex, int endStationIndex) const {
    return prices[endStationIndex] - prices[startStationIndex];
  }

  int buy(int trainNum, int startStationIndex, int endStationIndex, int num) {
    Seats &seats = *getSeats(trainNum, false);
    for (int i = startStationIndex; i < endStationIndex; i++) {
      if (seats[i] < num) {
        return -1;
      }
    }
    getSeats(trainNum, true);
    for (int i = startStationIndex; i < endStationIndex; i++) {
      seats[i] -= num;
    }
    return num * getPrice(startStationIndex, endStationIndex);
  }

  void refund(int trainNum, int startStationIndex, int endStationIndex, int num) {
    Seats &seats = *getSeats(trainNum, true);
    for (int i = startStationIndex; i < endStationIndex; i++) {
      seats[i] += num;
    }
  }
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
  PersistentMap<Train, 1000000> unreleasedTrainMap("unreleased_train");
  PersistentMap<Train, 1000000> releasedTrainMap("released_train");
  PersistentSet<Station, 1000000> stationMap("station");
  SimpleFile<TrainInfo, 4500000> trainDataFile("train_data");
  SimpleFile<Seats, 0> seatDataFile("seat_data");

  bool addTrain(const TrainInfo &trainInfo) {
    String20 index = trainInfo.trainID;
    if (unreleasedTrainMap.get(index).present || releasedTrainMap.get(index).present) {
      return false;
    }
    Train train{index, trainDataFile.write(trainInfo)};
    unreleasedTrainMap.insert(train);
    return true;
  }

  bool deleteTrain(const String20 &index) {
    return unreleasedTrainMap.erase(index);
  }

  bool releaseTrain(const String20 &index) {
    auto it = unreleasedTrainMap.get(index);
    if (!it.present) {
      return false;
    }
    Train train = *it.value;
    unreleasedTrainMap.erase(index);
    if (!releasedTrainMap.insert(train)) {
      throw;
    }
    TrainInfo *trainInfo = trainDataFile.get(train.trainData, true);
    for (int i = 0; i < trainInfo->stationNum; i++) {
      stationMap.insert(Station{trainInfo->stationNames[i], train.trainData, i});
    }
    return true;
  }

  Optional<TrainInfo *> getTrain(const String20 &index, bool dirty, bool shouldRelease) {
    if (!shouldRelease) {
      auto it1 = unreleasedTrainMap.get(index);
      if (it1.present) {
        return {trainDataFile.get(it1.value->trainData, dirty)};
      }
    }
    auto it2 = releasedTrainMap.get(index);
    if (it2.present) {
      return {trainDataFile.get(it2.value->trainData, dirty)};
    }
    return {};
  }

  void queryTicket(const String40 &from, const String40 &to, int date, bool isPrice) {
    auto it1 = stationMap.find({from, 0, 0});
    auto it2 = stationMap.find({to, 0, 0});
    priority_queue<Line> queue(isPrice ? Line::cmpPrice : Line::cmpTime);
    while (!it1.end() && it1->station == from && !it2.end() && it2->station == to) {
      if (it1->trainData == it2->trainData && it1->stationNum < it2->stationNum) {
        TrainInfo *trainInfo = trainDataFile.get(it1->trainData, false);
        int trainNum = trainInfo->findTrainNum(date, it1->stationNum);
        if (trainNum >= 0 && trainNum < trainInfo->totalCount) {
          Chrono departure = trainInfo->getDeparture(trainNum, it1->stationNum);
          Chrono arrival = trainInfo->getArrival(trainNum, it2->stationNum);
          int seat = trainInfo->getMaxSeat(trainNum, it1->stationNum, it2->stationNum);
          queue.push(Line{trainInfo->trainID,
                          from, departure,
                          to, arrival,
                          trainInfo->getPrice(it1->stationNum, it2->stationNum),
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

  struct TrainStationInfo {
    String40 station;
    int fromTrain;
    int fromIndex;
    int intersectionIndexFrom;
    auto operator<=>(const TrainStationInfo &rhs) const = default;
  };

  bool queryTransfer(const String40 &from, const String40 &to, int date, bool isPrice) {
    priority_queue<pair<Line, Line>> queue(isPrice ? Line::cmpPricePair : Line::cmpTimePair);
    auto it1 = stationMap.find({from, 0, 0});
    auto it2 = stationMap.find({to, 0, 0});
    set<TrainStationInfo> stationIndices;
    list<TrainInfo*> trainFromList;
    while (!it1.end() && it1->station == from) {
      TrainInfo *trainFrom = trainDataFile.get(it1->trainData, false);
      for (int intersectionIndexFrom = it1->stationNum + 1;
           intersectionIndexFrom < trainFrom->stationNum; intersectionIndexFrom++) {
        stationIndices.insert({trainFrom->stationNames[intersectionIndexFrom], (int)trainFromList.size(), it1->stationNum, intersectionIndexFrom});
      }
      trainFromList.push_back(trainFrom);
      it1++;
    }
    while (!it2.end() && it2->station == to) {
      TrainInfo *trainTo = trainDataFile.get(it2->trainData, false);
      for (int intersectionIndexTo = 0; intersectionIndexTo < it2->stationNum; intersectionIndexTo++) {
        const String40& intersection = trainTo->stationNames[intersectionIndexTo];
        auto range = stationIndices.lower_bound({intersection, 0, 0, 0});
        for (auto candidate = range; candidate!=stationIndices.end() && candidate->station == intersection; candidate++) {
          TrainInfo *trainFrom = trainFromList[candidate->fromTrain];
          if(trainFrom->trainID == trainTo->trainID) {
            continue;
          }
          int indexFrom = candidate->fromIndex;
          int intersectionIndexFrom = candidate->intersectionIndexFrom;
          int indexTo = it2->stationNum;
          int trainNumFrom = trainFrom->findTrainNum(date, indexFrom);
          if (trainNumFrom < 0 || trainNumFrom >= trainFrom->totalCount) {
            continue;
          }
          Chrono departureFrom = trainFrom->getDeparture(trainNumFrom, indexFrom);
          Chrono arrivalFrom = trainFrom->getArrival(trainNumFrom, intersectionIndexFrom);
          int trainNumTo = trainTo->findBestTrainNum(arrivalFrom, intersectionIndexTo);
          if (trainNumTo < 0) {
            continue;
          }
          Chrono departureTo = trainTo->getDeparture(trainNumTo, intersectionIndexTo);
          Chrono arrivalTo = trainTo->getArrival(trainNumTo, indexTo);
          queue.push({Line{trainFrom->trainID,
                           from, departureFrom,
                           intersection, arrivalFrom,
                           trainFrom->getPrice(indexFrom, intersectionIndexFrom),
                           trainFrom->getMaxSeat(trainNumFrom, indexFrom, intersectionIndexFrom),
                           arrivalFrom.toTick() - departureFrom.toTick()},
                      Line{trainTo->trainID,
                           intersection, departureTo,
                           to, arrivalTo,
                           trainTo->getPrice(intersectionIndexTo, indexTo),
                           trainTo->getMaxSeat(trainNumTo, intersectionIndexTo, indexTo),
                           arrivalTo.toTick() - departureTo.toTick()}});
        }
      }
      it2++;
    }
    if (queue.empty()) {
      return false;
    }
    std::cout << queue.top().first << '\n' << queue.top().second;
    return true;
  }
}

#endif