#include "ObjectId.h"
#include "JSMath.h"
#include "Date.h"
#include <exception>

bool ObjectId::_init = false;
int ObjectId::_index = 0;
byte ObjectId::_PROCESS_UNIQUE[5] = {0};

std::regex ObjectId::_checkForHexRegExp("^[0-9a-fA-F]{24}$");
bool ObjectId::cacheHexString = false;

void ObjectId::_checkInit() {
  if (!_init) {
    _index = (int)Math::floor((Math::random() * 0xffffff));
    for (int i = 0; i < 5; i++) {
      _PROCESS_UNIQUE[i] = (byte)Math::floor(Math::random() * 256);
    }
    _init = true;
  }
}

int ObjectId::_getInc() {
  return (ObjectId::_index = (ObjectId::_index + 1) % 0xffffff);
}

ObjectId::ObjectId() {
  _checkInit();
  const Buffer& buf = ObjectId::generate();
  for (int i = 0; i < 12; i++) {
    _id[i] = buf[i];
  }
  if (ObjectId::cacheHexString)
    __id = toHexString();
}

ObjectId::ObjectId(int id) {
  _checkInit();
  const Buffer& buf = ObjectId::generate(id);
  for (int i = 0; i < 12; i++) {
    _id[i] = buf[i];
  }
  if (ObjectId::cacheHexString)
    __id = toHexString();
}

ObjectId::ObjectId(const Buffer& id) {
  _checkInit();
  for (int i = 0; i < 12; i++) {
    _id[i] = id[i];
  }
  if (ObjectId::cacheHexString)
    __id = toHexString();
}

ObjectId::ObjectId(const ObjectId& id) {
  _checkInit();
  const byte* buf = id.getId();
  for (int i = 0; i < 12; i++) {
    _id[i] = buf[i];
  }
  if (ObjectId::cacheHexString)
    __id = id.getCachedId();
}

ObjectId::ObjectId(const String& id) {
  _checkInit();
  boolean valid = ObjectId::isValid(id);

  if (!valid && id != "") {
    throw std::exception(
      "Argument passed in must be a single String of 12 bytes or a string of 24 hex characters");
  } else if (valid && id.length() == 24) {
    for (int i = 0; i < 24; i += 2) {
      _id[i / 2] = (byte)strtol(id.substring(i, i + 2).toCString(), nullptr, 16);
    }
  } else if (id != "" && id.length() == 12) {
    for (int i = 0; i < 12; i++) {
      _id[i] = (byte)id.toCppString()[i];
    }
  } else {
    throw std::exception(
      "Argument passed in must be a single String of 12 bytes or a string of 24 hex characters");
  }

  if (ObjectId::cacheHexString)
    __id = toHexString();
}

bool ObjectId::isValid(const String& id) {
  if (id == "")
    return false;
  return id.length() == 12 || (id.length() == 24 && std::regex_match(id.toCString(), _checkForHexRegExp));
}

bool ObjectId::isValid(int64_t id) {
  return true;
}

bool ObjectId::isValid(const ObjectId& id) {
  return true;
}

bool ObjectId::isValid(const Buffer& id) {
  return id.length() == 12;
}

ObjectId ObjectId::createFromHexString(const String& str) {
  if (str.length() != 24) {
    throw std::exception("Argument passed in must be a single String of 24 hex characters");
  }

  byte array[12] = {0};

  for (int i = 0; i < 24; i += 2) {
    array[i / 2] = (byte)strtol(str.substring(i, i + 2).toCString(), nullptr, 16);
  }

  return Buffer::from(array, 12);
}

ObjectId ObjectId::createFromTime(int time) {
  byte buffer[12] = {0};

  buffer[3] = (byte)(time & 0xff);
  buffer[2] = (byte)((time >> 8) & 0xff);
  buffer[1] = (byte)((time >> 16) & 0xff);
  buffer[0] = (byte)((time >> 24) & 0xff);

  return Buffer::from(buffer, 12);
}

Buffer ObjectId::generate() {
  _checkInit();
  int time = (int)(Date::now() / 1000);
  return generate(time);
}

Buffer ObjectId::generate(int time) {
  _checkInit();
  int inc = ObjectId::_getInc();
  byte buffer[12] = {0};

  buffer[3] = (byte)(time & 0xff);
  buffer[2] = (byte)((time >> 8) & 0xff);
  buffer[1] = (byte)((time >> 16) & 0xff);
  buffer[0] = (byte)((time >> 24) & 0xff);

  buffer[4] = _PROCESS_UNIQUE[0];
  buffer[5] = _PROCESS_UNIQUE[1];
  buffer[6] = _PROCESS_UNIQUE[2];
  buffer[7] = _PROCESS_UNIQUE[3];
  buffer[8] = _PROCESS_UNIQUE[4];

  buffer[11] = (byte)(inc & 0xff);
  buffer[10] = (byte)((inc >> 8) & 0xff);
  buffer[9] = (byte)((inc >> 16) & 0xff);

  return Buffer::from(buffer, 12);
}

int ObjectId::getGenerationTime() const {
  return (_id[3] & 0xff) | ((_id[2] & 0xff) << 8) | ((_id[1] & 0xff) << 16) | ((_id[0] & 0xff) << 24);
}

void ObjectId::setGenerationTime(int value) {
  _id[3] = (byte)(value & 0xff);
  _id[2] = (byte)((value >> 8) & 0xff);
  _id[1] = (byte)((value >> 16) & 0xff);
  _id[0] = (byte)((value >> 24) & 0xff);
}

bool ObjectId::equals(const ObjectId& otherId) const {
  return toString() == otherId.toString();
}

bool ObjectId::equals(const String& otherId) const {
  if (ObjectId::isValid(otherId) && otherId.length() == 12) {
    return otherId == toString("binary");
  }

  if (ObjectId::isValid(otherId) && otherId.length() == 24) {
    return otherId.toLowerCase() == toHexString();
  }

  return false;
}

const byte* ObjectId::getId() const {
  return _id;
}

const String& ObjectId::getCachedId() const {
  return __id;
}

String ObjectId::toString() const {
  return toHexString();
}

String ObjectId::toString(const String& type) const {
  if (type == "binary") {
    String res = "";
    for (int i = 0; i < 12; i++) {
      res += ((char)(_id[i]));
    }
    return res;
  }
  return toString();
}

String ObjectId::toHexString() const {
  return Buffer::from(_id, 12).toString("hex");
}
