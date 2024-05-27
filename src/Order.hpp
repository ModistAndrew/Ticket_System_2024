//
// Created by zjx on 2024/5/27.
//

#ifndef TICKETSYSTEM2024_ORDER_HPP
#define TICKETSYSTEM2024_ORDER_HPP

#include "Util.hpp"
#include "PersistentMultiMap.hpp"
#include "PersistentSet.hpp"

struct Order {
  String20 index; //user ID
  String20 trainID;
  int status; //0 for success, 1 for pending, 2 for refunded
  int trainNum;
  int from;
  int to;
  int num;
};

struct OrderQueue {
  String20 trainID;
  int trainNum;
  int leafPos; //negative
  int pos; //negative
  auto operator<=>(const OrderQueue &rhs) const = default;
};

namespace Orders {
  PersistentMultiMap<Order> orderMap("order");
  PersistentSet<OrderQueue> orderQueueMap("order_queue");

  void addOrder(Order &order) { //success or pending
    orderMap.insert(order);
    if(order.status == 1) {
      auto it = orderMap.find(order.index); //point to the last order which has just been inserted
      orderQueueMap.insert(OrderQueue{order.trainID, order.trainNum, -it.getLeafPos(), -it.getPos()});
    }
  }
}
#endif //TICKETSYSTEM2024_ORDER_HPP
