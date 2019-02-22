#ifndef __JSARRAY_HPP__
#define __JSARRAY_HPP__

#include <deque>
#include <initializer_list>
#include "JSString.h"

template <typename T>
class Array {
public:

  virtual ~Array() {}
  Array(const std::initializer_list<T>& list) {
    _value = list;
  }

  Array() {}

  Array(const Array<T>& other) {
    _value = other._value;
  }

  int length() const {
    return static_cast<int>(_value.size());
  }

  const T& operator[](int index) const {
    return _value[index];
  }

  T& operator[](int index) {
    return _value[index];
  }

  bool operator==(const Array<T>& other) {
    return _value == other._value;
  }

  const Array<T>& operator=(const Array<T>& other) {
    if ((*this) == other) {
      return *this;
    }
    _value = other._value;
    return *this;
  }

  Array<T> concat(const T& value) const {
    Array<T> res = *this;
    res.push(value);
    return res;
  }

  Array<T> concat(const Array<T>& arr) const {
    Array<T> res = *this;
    for (int i = 0; i < arr.length(); i++) {
      res.push(arr[i]);
    }
    return res;
  }

  template <typename... Arg>
  Array<T> concat(const T& arg, Arg... args) const {
    Array<T> tmp = this->concat(arg);
    return tmp.concat(args...);
  }

  template <typename... Arg>
  Array<T> concat(const Array<T>& arg, Arg... args) const {
    Array<T> tmp = this->concat(arg);
    return tmp.concat(args...);
  }

  Array<T> splice(int start) {
    return splice(start, length() - start);
  }

  Array<T> splice(int start, int deleteCount) {
    Array<T> res;
    for (int i = 0; i < deleteCount; i++) {
      res.push(_value[start + i]);
    }
    _value.erase(_value.begin() + start, _value.begin() + start + deleteCount);
    return res;
  }

  Array<T> splice(int start, int deleteCount, const T& item) {
    Array<T> res = splice(start, deleteCount);
    _value.insert(_value.begin() + start, item);
    return res;
  }

  template <typename... Arg>
  Array<T> splice(int start, int deleteCount, const T& item, Arg... args) {
    Array<T> res = splice(start, deleteCount);
    splice(start, 0, item);
    splice(start + 1, 0, args...);
    return res;
  }

  String toString() const {
    String res = join(",");
    return res == "" ? "[object Array]" : res;
  }

  String join() const {
    return join("");
  }

  String join(const String& sep) const {
    int len = length();
    if (std::is_same<T, String>::value || std::is_same<T, std::string>::value) {
      String res = "";
      for (int i = 0; i < len - 1; i++) {
        res += (_value[i] + sep.toCppString());
      }
      res += _value[len - 1];
      return res;
    }

    // if (std::is_same<T, int>::value || std::is_same<T, long>::value ||
    //     std::is_same<T, long long>::value || std::is_same<T, unsigned>::value ||
    //     std::is_same<T, unsigned long>::value ||
    //     std::is_same<T, unsigned long long>::value ||
    //     std::is_same<T, float>::value || std::is_same<T, double>::value ||
    //     std::is_same<T, long double>::value) {
    //   String res = "";
    //   for (int i = 0; i < len; i++) {
    //     res += (std::to_string((long double)(_value[i])) + sep.toCppString());
    //   }
    //   res += _value[len - 1];
    //   return res;
    // }

    return "";
  }

  int push(const T& element) {
    _value.push_back(element);
    return length();
  }

  int unshift(const T& element) {
    _value.push_front(element);
    return length();
  }

  T shift() {
    T res(_value[0]);
    _value.pop_front();
    return res;
  }

  T pop() {
    T res(_value[_value.size() - 1]);
    _value.pop_back();
    return res;
  }

  Array<T> slice() const {
    return this->slice(0, length());
  }

  Array<T> slice(int start) const {
    return this->slice(start, length());
  }

  Array<T> slice(int start, int end) const {
    int _length = length();
    end--;
    start = start < 0 ? (_length + (start % _length)) : start % _length;
    end = end < 0 ? (_length + (end % _length)) : end % _length;
    if (end < start) {
      int tmp = end;
      end = start;
      start = tmp;
    }

    int len = end - start + 1;

    Array<T> res;

    if (len <= 0) return res;

    for (int i = 0; i < len; i++) {
      res.push((*this)[start + i]);
    }
    return res;
  }

  int indexOf(const T& element) const {
    for (int i = 0; i < _value.size(); i++) {
      if (element == _value[i]) {
        return i;
      }
    }
    return -1;
  }

  int lastIndexOf(const T& element) const {
    for (int i = _value.size() - 1; i >= 0; i--) {
      if (element == _value[i]) {
        return i;
      }
    }
    return -1;
  }

  bool includes(const T& element) const {
    return -1 != indexOf(element);
  }

  template <typename F>
  void forEach(F callback) {
    for (int i = 0; i < _value.size(); i++) {
      callback(_value[i], i, (*this));
    }
  }

  template <typename U, typename F>
  Array<U> map(F callback) {
    Array<U> res = {};
    for (int i = 0; i < _value.size(); i++) {
      res.push(callback(_value[i], i, (*this)));
    }
    return res;
  }

  std::deque<T>& getContainer() const {
    return _value;
  }

private:
  std::deque<T> _value;
};

#endif
