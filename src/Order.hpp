//
// Created by zjx on 2024/5/27.
//

#ifndef TICKETSYSTEM2024_ORDER_HPP
#define TICKETSYSTEM2024_ORDER_HPP

#include "Util.hpp"
#include "PersistentMultiMap.hpp"
#include "PersistentSet.hpp"
#include "Train.hpp"

struct Order {
  String20 userID;
  String20 trainID;
  int trainNum;
  int status; //0 for success, 1 for pending, 2 for refunded
  int fromId;
  String40 from;
  Chrono departureTime;
  int toId;
  String40 to;
  Chrono arrivalTime;
  int price;
  int num;
  using INDEX = String20;

  INDEX index() const {
    return userID;
  }

  std::string getStatus() const {
    if (status == 0) {
      return "[success]";
    } else if (status == 1) {
      return "[pending]";
    } else {
      return "[refunded]";
    }
  }

  friend std::ostream &operator<<(std::ostream &out, const Order &b) {
    return out << b.getStatus() << ' ' << b.trainID << ' ' <<
    b.from << ' ' << b.departureTime << " -> " << b.to << ' ' << b.arrivalTime << ' ' << b.price << ' ' << b.num;
  }
};

struct OrderQueue {
  pair<String20, int> train; //trainID, trainNum
  using INDEX = pair<String20, int>;
  INDEX index() const {
    return train;
  }
  String20 userID;
  int tick;
};

namespace Orders {
  PersistentMultiMap<Order> orderMap("order");
  PersistentMultiMap<OrderQueue> orderQueueMap("order_queue");

  void addOrder(Order &order) { //success or pending
    int tick = orderMap.pushFront(order);
    if (order.status == 1) {
      orderQueueMap.pushBack(OrderQueue{{order.trainID, order.trainNum}, order.userID, tick});
    }
  }

  void printOrders(const String20 &id) {
    auto it1 = orderMap.find(id); //find the first order of the user
    auto it2 = it1;
    int count = 0;
    while (!it1.end() && it1->val.index() == id) {
      count++;
      ++it1;
    }
    std::cout << count;
    for (int i = 0; i < count; i++) {
      std::cout << '\n' << it2->val;
      ++it2;
    }
  }

  bool refundOrder(const String20 &id, int num) {
    auto itNow = orderMap.find(id);
    while (!itNow.end() && itNow->val.index() == id && --num) {
      ++itNow;
    }
    if (itNow.end() || itNow->val.index() != id) {
      return false;
    }
    itNow.markDirty();
    Order &orderNow = itNow->val;
    if(orderNow.status == 1) {
      orderNow.status = 2;
      return true;
    }
    if(orderNow.status == 2) {
      return false;
    }
    orderNow.status = 2;
    auto train = Trains::getTrain(orderNow.trainID, true, true);
    if (!train.present) {
      throw;
    }
    TrainInfo &trainInfo = *train.value;
    trainInfo.refund(orderNow.trainNum, orderNow.fromId, orderNow.toId, orderNow.num);
    auto itQueue = orderQueueMap.find({orderNow.trainID, orderNow.trainNum});
    list<int> toErase;
    while (!itQueue.end() && itQueue->val.train.first == orderNow.trainID && itQueue->val.train.second == orderNow.trainNum) {
      auto orderPendingRef = orderMap.get(itQueue->val.userID, itQueue->val.tick);
      if(!orderPendingRef.second) {
        throw;
      }
      Order &orderPending = orderPendingRef.first->val;
      if (orderPending.status == 1) {
        if (trainInfo.buy(orderPending.trainNum, orderPending.fromId, orderPending.toId, orderPending.num) >= 0) {
          orderPendingRef.first.markDirty();
          orderPending.status = 0;
        }
      }
      if (orderPending.status != 1) {
        toErase.push_back(itQueue->val.tick);
      }
      ++itQueue;
    }
    for (auto i: toErase) {
      orderQueueMap.erase({orderNow.trainID, orderNow.trainNum}, i);
    }
    return true;
  }
}
#endif
