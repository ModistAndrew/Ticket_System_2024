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
      return "success";
    } else if (status == 1) {
      return "pending";
    } else {
      return "refunded";
    }
  }

  friend std::ostream &operator<<(std::ostream &out, const Order &b) {
    return out << b.getStatus() << ' ' << b.trainID << ' ' <<
               b.from << ' ' << b.departureTime << " -> " << b.to << ' ' << b.arrivalTime << ' ' << b.price << ' '
               << b.num;
  }
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
    if (order.status == 1) {
      auto it = orderMap.find(order.index); //point to the last order which has just been inserted
      orderQueueMap.insert(OrderQueue{order.trainID, order.trainNum, -it.getLeafPos(), -it.getPos()});
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
    std::cout << count << '\n';
    for (int i = 0; i < count; i++) {
      std::cout << *it2;
      if (i != count - 1) {
        std::cout << '\n';
        ++it2;
      }
    }
  }
}
#endif //TICKETSYSTEM2024_ORDER_HPP
