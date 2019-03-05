#ifndef __OBJECT_ID_H__
#define __OBJECT_ID_H__

#include <regex>
#include "JSString.h"
#include "JSBuffer.h"

class ObjectId {
private:
  static int _index;
  static byte _PROCESS_UNIQUE[5];
  static std::regex _checkForHexRegExp;
  static bool _init;
  static void _checkInit();

  byte _id[12];
  String __id;

  static int _getInc();
public:
  static bool cacheHexString;

  ObjectId();
  ObjectId(int);
  ObjectId(const Buffer&);
  ObjectId(const ObjectId&);
  ObjectId(const String&);

  static bool isValid(const String&);
  static bool isValid(int64_t);
  static bool isValid(const ObjectId&);
  static bool isValid(const Buffer&);

  static ObjectId createFromHexString(const String&);
  static ObjectId createFromTime(int);

  static Buffer generate();
  static Buffer generate(int);

  int getGenerationTime() const;
  void setGenerationTime(int);

  bool equals(const ObjectId&) const;
  bool equals(const String&) const;

  const byte* getId() const;
  const String& getCachedId() const;

  String toString() const;
  String toString(const String&) const;
  String toHexString() const;
};

#endif