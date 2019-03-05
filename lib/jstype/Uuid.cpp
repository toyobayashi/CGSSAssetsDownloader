#include "Uuid.h"
#include "JSMath.h"


Buffer Uuid::toBuffer() {
  return Buffer::from(_buf, 16);
}

String Uuid::toString() {
  Buffer buf = toBuffer();
  String hex = buf.toString("hex");
  return hex.substring(0, 8) + "-" + hex.substring(8, 12) + "-" + hex.substring(12, 16) + "-" + hex.substring(16, 20) + "-" + hex.substring(20);
}

const byte * Uuid::toCBuffer() {
  return _buf;
}

Uuid::Uuid() {
  for (int i = 0; i < 16; i++) {
    _buf[i] = (byte)Math::floor(Math::random() * 256);
  }

  _buf[6] = (byte)((_buf[6] & 0x0f) | 0x40);
  _buf[8] = (byte)((_buf[8] & 0x3f) | 0x80);
}
