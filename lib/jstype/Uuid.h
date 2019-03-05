#ifndef __UUID_H__
#define __UUID_H__

#include "JSBuffer.h"
#include "JSString.h"

class Uuid {
public:
  Buffer toBuffer();
  String toString();
  const byte* toCBuffer();
  Uuid();

private:
  byte _buf[16];
};

#endif
