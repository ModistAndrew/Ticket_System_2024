#ifndef SJTU_EXCEPTIONS_HPP
#define SJTU_EXCEPTIONS_HPP

#include <string>

struct Error {
  const std::string message;
};

struct IndexOutOfBound : public Error {
  IndexOutOfBound() : Error("Index out of bound") {}
};

struct InvalidIterator : public Error {
  InvalidIterator() : Error("Invalid iterator") {}
};

struct FileSizeExceeded : public Error {
  FileSizeExceeded() : Error("File size exceeded") {}
};

#endif