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
  int status;
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

  void addOrder(const Order &order) {
    orderMap.insert(order);
    orderQueueMap.insert(OrderQueue{order.trainID, order.trainNum, -1, -1});
  }
}
#endif //TICKETSYSTEM2024_ORDER_HPP
