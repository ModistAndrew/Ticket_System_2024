//
// Created by zjx on 2024/5/26.
//

#ifndef TICKETSYSTEM2024_ACCOUNT_HPP
#define TICKETSYSTEM2024_ACCOUNT_HPP

#include "Util.hpp"
#include "PersistentMap.hpp"

struct Account {
  String20 userID;
  String30 password;
  String20 name;
  String30 mailAddr;
  int privilege;
  using INDEX = String20;

  INDEX index() const {
    return userID;
  }

  Account(const std::string &index, const std::string &password, const std::string &name, const std::string &mailAddr, int privilege) :
    userID(index), password(password), name(name), mailAddr(mailAddr), privilege(privilege) {}

  Account() = default;

  friend std::ostream &operator<<(std::ostream &out, const Account &b) {
    return out << b.userID << ' ' << b.name << ' ' << b.mailAddr << ' ' << b.privilege;
  }
};

namespace AccountStorage {
  PersistentMap<Account> accountMap("accounts");

  bool empty() {
    return accountMap.empty();
  }

  bool add(const Account &account) {
    return accountMap.insert(account);
  }

  Optional<Account> get(const String20 &index) {
    auto ret = accountMap.get(index);
    return ret.second ? Optional<Account>(*ret.first) : Optional<Account>();
  }

  void modify(const Account &newAccount); // modify the account with the same index. make sure the account exists. may also change logged account
}

namespace Accounts {
  map<String20, Account> currentAccounts;

  Optional<Account> getLogged(const String20 &index) {
    auto it = currentAccounts.find(index);
    return it == currentAccounts.end() ? Optional<Account>() : Optional<Account>(it->second);
  }

  bool login(const String20 &index, const String30 &password) {
    auto account = AccountStorage::get(index);
    if (!account.present || account.value.password != password) {
      return false;
    }
    return currentAccounts.insert({index, account.value}).second;
  }

  bool logout(const String20 &index) {
    auto it = currentAccounts.find(index);
    if (it == currentAccounts.end()) {
      return false;
    }
    currentAccounts.erase(it);
    return true;
  }
}

void AccountStorage::modify(const Account &newAccount) {
  if(accountMap.erase(newAccount.userID)) {
    accountMap.insert(newAccount);
  } else {
    throw;
  }
  if(Accounts::logout(newAccount.userID)) {
    Accounts::currentAccounts.insert({newAccount.userID, newAccount});
  }
}
#endif
