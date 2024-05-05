//
// Created by zjx on 2024/4/26.
//

#ifndef TICKET_SYSTEM_2024_UTIL_HPP
#define TICKET_SYSTEM_2024_UTIL_HPP

#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <cmath>

template<typename T>
T* lower_bound(T* first, T* last, const T& val) {
  while (first < last) {
    T* mid = first + (last - first) / 2;
    if (*mid < val) {
      first = mid + 1;
    } else {
      last = mid;
    }
  }
  return first;
}

template<typename T>
T* upper_bound(T* first, T* last, const T& val) {
  while (first < last) {
    T* mid = first + (last - first) / 2;
    if (*mid <= val) {
      first = mid + 1;
    } else {
      last = mid;
    }
  }
  return first;
}
#endif //TICKET_SYSTEM_2024_UTIL_HPP
