//
// Created by zjx on 2024/5/27.
//

#ifndef TICKETSYSTEM2024_ORDER_HPP
#define TICKETSYSTEM2024_ORDER_HPP

#include "Util.hpp"
#include "PersistentMultiMap.hpp"

struct OrderInfo {
  int status; //0: pending, 1: paid, 2: refunded
  std::string trainID;
  int date;
  int from;
  int to;
  int num;
  int next; //next order in the same user. store as a linked list
};

struct Order {
  String20 index; //user ID
  int location; //where order data is stored
};

PersistentMultiMap<Order, OrderInfo, 1000000> orderMap("order", "order");

#endif //TICKETSYSTEM2024_ORDER_HPP
