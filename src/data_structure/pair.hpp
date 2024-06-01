//
// Created by zjx on 2024/6/1.
//

#ifndef TICKETSYSTEM2024_PAIR_HPP
#define TICKETSYSTEM2024_PAIR_HPP
template<class T1, class T2>
struct pair {
  T1 first;
  T2 second;

  constexpr pair() : first(), second() {}

  pair(const pair &other) = default;

  pair(pair &&other) = default;

  pair &operator=(const pair &other) {
    if (this != &other) {
      first = other.first;
      second = other.second;
    }
    return *this;
  }

  pair(const T1 &x, const T2 &y) : first(x), second(y) {}

  template<class U1, class U2>
  pair(U1 &&x, U2 &&y) : first(x), second(y) {}

  template<class U1, class U2>
  explicit pair(const pair<U1, U2> &other) : first(other.first), second(other.second) {}

  template<class U1, class U2>
  explicit pair(pair<U1, U2> &&other) : first(other.first), second(other.second) {}

  auto operator<=>(const pair &) const = default;
};
#endif //TICKETSYSTEM2024_PAIR_HPP
