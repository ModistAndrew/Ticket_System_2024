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
  String20 index; //user ID
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
               b.from << ' ' << b.departureTime << " -> " << b.to << ' ' << b.arrivalTime << ' ' << b.price << ' '
               << b.num;
  }

  bool refund() { //refund, return whether the order is successfully refunded
    if (status == 0) {
      status = 2;
      return true;
    }
    status = 2;
    return false;
  }

  auto operator<=>(const Order &rhs) const {
    if(auto cmp = trainID <=> rhs.trainID; cmp != 0) {
      return cmp;
    }
    if(auto cmp = trainNum <=> rhs.trainNum; cmp != 0) {
      return cmp;
    }
  }
};

namespace Orders {
  PersistentMultiMap<Order> orderMap("order");
  PersistentSet<OrderQueue> orderQueueMap("order_queue");

  void addOrder(Order &order) { //success or pending
    orderMap.insert(order);
    if (order.status == 1) {
      auto it = orderMap.find(order.index); //point to the last order which has just been inserted
      orderQueueMap.insert(OrderQueue{order.trainID, order.trainNum, it.getLeafPos(), it.getPos()});
    }
  }

  void printOrders(const String20 &id) {
    auto it1 = orderMap.find(id);
    auto it2 = it1;
    int count = 0;
    while (!it1.end() && it1->index == id) {
      count++;
      ++it1;
    }
    std::cout << count;
    for (int i = 0; i < count; i++) {
      std::cout << '\n' << *it2;
      ++it2;
    }
  }

  bool refundOrder(const String20 &id, int num) {
    auto itNow = orderMap.find(id);
    while (!itNow.end() && itNow->index == id && --num) {
      ++itNow;
    }
    if (itNow.end() || itNow->index != id) {
      return false;
    }
    itNow.markDirty();
    if(!itNow->refund()) {
      return true;
    }
    auto train = Trains::getTrain(itNow->trainID, true, true);
    if (!train.present) {
      throw;
    }
    TrainInfo &trainInfo = train.value;
    trainInfo.refund(itNow->trainNum, itNow->fromId, itNow->toId, itNow->num);
    auto itQueue = orderQueueMap.find({itNow->trainID, itNow->trainNum, 0, 0});
    while (!itQueue.end() && itQueue->trainID == itNow->trainID && itQueue->trainNum == itNow->trainNum) {
      auto itQueueRef = orderMap.create(itQueue->leafPos, itQueue->pos);
      if (itQueueRef->status == 1) {
        if (trainInfo.buy(itQueueRef->trainNum, itQueueRef->fromId, itQueueRef->toId, itQueueRef->num) >= 0) {
          itQueueRef.markDirty();
          itQueueRef->status = 0;
          return true;
        }
      }
      ++itQueue;
    }
    return true;
  }
}
#endif //TICKETSYSTEM2024_ORDER_HPP
