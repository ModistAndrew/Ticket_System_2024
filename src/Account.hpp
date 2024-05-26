//
// Created by zjx on 2024/5/26.
//

#ifndef TICKETSYSTEM2024_ACCOUNT_HPP
#define TICKETSYSTEM2024_ACCOUNT_HPP
#include "Util.hpp"
#include "PersistentMap.hpp"
struct Account {
  String20 index;
  String30 password;
  String20 name;
  String30 mailAddr;
  int privilege;

  String20 getIndex() const {
    return index;
  }

  friend std::ostream &operator<<(std::ostream &out, const Account &b) {
    return out << b.index << ' ' << b.name << ' ' << b.mailAddr << ' ' << b.privilege;
  }
};

namespace Accounts {
  PersistentMap<Account> accountMap("accounts");

  bool add(const Account &account) {
    return accountMap.insert(account);
  }

  Optional<Account> get(const String20 &index) {
    auto ret = accountMap.find(index);
    return ret.second ? Optional<Account>(*ret.first) : Optional<Account>();
  }

  bool remove(const String20 &index) {
    return accountMap.erase(index);
  }
}
#endif //TICKETSYSTEM2024_ACCOUNT_HPP
