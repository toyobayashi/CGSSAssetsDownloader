#ifndef __JSOBJECT_HPP__
#define __JSOBJECT_HPP__

#include "JSArray.hpp"
#include "JSString.h"
#include <map>

template <typename T>
class Object {
public:
  static Array<String> keys(const Object<T>& obj) {
    Array<String> res;
    auto it = obj._data.begin();
    for (; it != obj._data.end(); it++) {
      res.push(it->first);
    }
    return res;
  }

  static Array<T> values(const Object<T>& obj) {
    Array<T> res;
    auto it = obj._data.begin();
    for (; it != obj._data.end(); it++) {
      res.push(it->second);
    }
    return res;
  }

  static Object<T>& assign(Object<T>& target, const Object<T>& source) {
    auto keys = Object<T>::keys(source);
    for (int i = 0; i < keys.length(); i++) {
      target[keys[i]] = source[keys[i]];
    }
    return target;
  }

  template <typename... Args>
  static Object<T>& assign(Object<T>& target, const Object<T>& source, Args... args) {
    Object<T>& tar = assign(target, source);
    return assign(tar, args...);
  }

  static const std::map<String, T>& entries(const Object<T>& obj) {
    return obj._data;
  }

  static Object fromEntries(const std::map<String, T>& iterable) {
    return iterable;
  }

  virtual String toString() const {
    return "[object Object]";
  }

  Object() {}
  Object(const std::map<String, T>& other) {
    _data = other;
  }
  Object(const Object<T>& other) {
    _data = other._data;
  }

  bool has(const String& key) {
    std::string k = key.toCppString();
    return _data.find(k) != _data.end();
  }

  void del(const String& key) {
    std::string k = key.toCppString();
    auto it = _data.find(k);
    if (it != _data.end()) {
      _data.erase(it);
    }
  }

  const Object<T>& operator=(const Object<T>& other) {
    if (this == &other) {
      return *this;
    }
    _data = other._data;
    return *this;
  }

  const T& operator[](const String& key) const {
    std::string k = key.toCppString();
    return _data[k];
  }

  T& operator[](const String& key) {
    std::string k = key.toCppString();
    return _data[k];
  }
private:
  std::map<String, T> _data;
};

#endif