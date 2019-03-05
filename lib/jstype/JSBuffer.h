#ifndef __JS_BUFFER_H__
#define __JS_BUFFER_H__

#include "JSString.h"
#include "JSArray.hpp"
#include <vector>

typedef unsigned char byte;

class Buffer {
 public:
  virtual ~Buffer();
  static Buffer alloc(int);
  static Buffer from(Buffer&&);
  static Buffer from(const Buffer&);
  static Buffer from(const Array<byte>&);
  static Buffer from(const byte*, int);
  static Buffer from(const String&);
  static Buffer from(const String&, const String&);
  const byte* buffer() const;
  Buffer slice(int) const;
  Buffer slice(int, int) const;
  Buffer concat(const Buffer&) const;
  int length() const;
  String toString() const;
  String toString(const String&) const;
  std::vector<byte> toVector() const;

  const byte& operator[](int) const;
  byte& operator[](int);

  Buffer(const Buffer&);
  Buffer(Buffer&&);

 private:
  Buffer();
  Buffer(int);
  Buffer(const Array<byte>&);
  Buffer(const byte*, int);
  Buffer(const String&);
  Buffer(const String&, const String&);
  
  byte* _buffer;
  int _length;
  static char _map[16];
};

#endif
