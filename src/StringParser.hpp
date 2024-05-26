//
// Created by zjx on 2024/5/26.
//

#ifndef TICKETSYSTEM2024_STRINGPARSER_HPP
#define TICKETSYSTEM2024_STRINGPARSER_HPP
#include <string>
#include <vector>
using std::vector;

int parseInt(const std::string &s) {
  int x = 0;
  bool neg = false;
  for (char c: s) {
    if (c == '-') {
      neg = true;
      continue;
    }
    x = x * 10 + c - '0';
  }
  return neg ? -x : x;
}

std::string toStringInt(int x, int minLength = 0) {
  if (x == 0) {
    return "0";
  }
  std::string ret;
  if (x < 0) {
    ret.push_back('-');
    x = -x;
  }
  std::string tmp;
  while (x) {
    tmp.push_back(x % 10 + '0');
    x /= 10;
  }
  while (tmp.length() < minLength) {
    tmp.push_back('0');
  }
  for (int i = tmp.length() - 1; i >= 0; i--) {
    ret.push_back(tmp[i]);
  }
  return ret;
}

//vector shouldn't be empty and there should be no empty string
vector<std::string> parseVector(const std::string &s, char delim) {
  vector<std::string> ret;
  std::string tmp;
  for (char c: s) {
    if (c == delim) {
      ret.push_back(tmp); //we accept empty string
      tmp.clear();
    } else {
      tmp.push_back(c);
    }
  }
  ret.push_back(tmp);
  return ret;
}

std::string toStringVector(const vector<std::string> &v, char delim) {
  std::string ret;
  for (int i = 0; i < v.size(); i++) {
    if (i) {
      ret += delim;
    }
    ret += v[i];
  }
  return ret;
}

//map empty vector to "_"
vector<int> parseIntVector(const std::string &s, char delim = '|') {
  if(s == "_") {
    return {};
  }
  vector<int> ret;
  std::string tmp;
  for (char c: s) {
    if (c == delim) {
      ret.push_back(parseInt(tmp));
      tmp.clear();
    } else {
      tmp.push_back(c);
    }
  }
  ret.push_back(parseInt(tmp));
  return ret;
}

std::string toStringIntVector(const vector<int> &v, char delim = '|') {
  if(v.empty()) {
    return "_";
  }
  std::string ret;
  for (int i = 0; i < v.size(); i++) {
    if (i) {
      ret += delim;
    }
    ret += toStringInt(v[i]);
  }
  return ret;
}

//length is the length of each number. every number should >= 0
vector<int> parseFixedIntVector(int length, const std::string &s) {
  vector<int> ret;
  for (int i = 0; i < s.length() / length; i++) {
    ret.push_back(parseInt(s.substr(i * length, length)));
  }
  return ret;
}

std::string toFixedStringIntVector(int length, const vector<int> &v) {
  std::string ret;
  for (int i : v) {
    ret += toStringInt(i, length);
  }
  return ret;
}

int parseTime(const std::string &s) { //00:00 to 23:59
  return ((s[0] - '0') * 10 + (s[1] - '0')) * 60 + ((s[3] - '0') * 10 + (s[4] - '0'));
}

std::string toStringTime(int x) {
  std::string ret;
  int hour = x / 60;
  int minute = x % 60;
  ret.push_back(hour / 10 + '0');
  ret.push_back(hour % 10 + '0');
  ret.push_back(':');
  ret.push_back(minute / 10 + '0');
  ret.push_back(minute % 10 + '0');
  return ret;
}

int parseDate(const std::string &s) { //06-01 to 12-31
  int day = (s[3] - '0') * 10 + (s[4] - '0');
  if (s[1] == '6') {
    return day - 1;
  } else if (s[1] == '7') {
    return day + 29;
  } else if (s[1] == '8') {
    return day + 60;
  } else if (s[1] == '9') {
    return day + 91;
  } else if (s[1] == '0') {
    return day + 121;
  } else if (s[1] == '1') {
    return day + 152;
  } else  {
    return day + 182;
  }
}


std::string toStringDate(int x) { //06-01 to 12-31
  std::string ret;
  if (x < 30) {
    ret = "06-";
    x += 1;
  } else if (x < 61) {
    ret = "07-";
    x -= 29;
  } else if (x < 92) {
    ret = "08-";
    x -= 60;
  } else if (x < 122) {
    ret = "09-";
    x -= 91;
  } else if (x < 153) {
    ret = "10-";
    x -= 121;
  } else if (x < 183) {
    ret = "11-";
    x -= 152;
  } else {
    ret = "12-";
    x -= 182;
  }
  ret.push_back(x / 10 + '0');
  ret.push_back(x % 10 + '0');
  return ret;
}

#endif //TICKETSYSTEM2024_STRINGPARSER_HPP
