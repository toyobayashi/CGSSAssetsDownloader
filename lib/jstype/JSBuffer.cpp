#include "JSBuffer.h"
#include "../base64/b64.h"

char Buffer::_map[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                         '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

Buffer::~Buffer() {
  delete[] _buffer;
  _buffer = nullptr;
  _length = 0;
}
Buffer::Buffer() {
  _buffer = nullptr;
  _length = 0;
}

Buffer::Buffer(int size) {
  _buffer = new byte[size]{0};
  _length = size;
}

Buffer::Buffer(const byte* buf, int size) {
  if (size <= 0) {
    _buffer = nullptr;
    _length = 0;
    return;
  }

  _buffer = new byte[size]{0};

	for (int i = 0; i < size; i++) {
		_buffer[i] = buf[i];
	}
	_length = size;
}

Buffer::Buffer(const Buffer& buffer) {
  const byte* buf = buffer._buffer;
  const int length = buffer._length;
  this->_buffer = new byte[length]{0};

  for (int i = 0; i < length; i++) {
    this->_buffer[i] = buf[i];
  }
  this->_length = length;
}

Buffer::Buffer(Buffer&& buffer) {
  _buffer = buffer._buffer;
  _length = buffer._length;
  buffer._buffer = nullptr;
  buffer._length = 0;
}

Buffer::Buffer(const String& str) {
  const char* buf = str.toCString();
  const int length = str.byteLength();
  _buffer = new byte[length]{0};

  for (int i = 0; i < length; i++) {
    _buffer[i] = (byte)buf[i];
  }
  _length = length;
}

Buffer::Buffer(const String& str, const String& encoding) {
  if (encoding == "base64") {
    int length;
    unsigned char* dec =
        b64_decode_ex(str.toCString(), str.byteLength(), (size_t*)(&length));
    _buffer = new byte[length]{0};
    for (int i = 0; i < length; i++) {
      _buffer[i] = dec[i];
    }
    _length = length;
    free(dec);
    dec = nullptr;
  } else if (encoding == "hex") {
    int byteLength = str.byteLength();
    if (byteLength % 2 != 0) {
      _buffer = nullptr;
      _length = 0;
      return;
    }

    const int length = byteLength / 2;
    _buffer = new byte[length]{0};
    for (int i = 0; i < length; i++) {
      _buffer[i] = (byte)strtol(str.substring(i * 2, (i * 2) + 2).toCString(), NULL, 16);
    }
    _length = length;

	} else {
    const char* buf = str.toCString();
    const int length = str.byteLength();
    _buffer = new byte[length]{0};

    for (int i = 0; i < length; i++) {
      _buffer[i] = (byte)buf[i];
    }
    _length = length;
  }
}

Buffer Buffer::from(const String& str) {
  return str;
}

Buffer Buffer::from(const Buffer& buffer) {
  return buffer;
}

Buffer Buffer::from(const byte* buf, int size) {
  return Buffer(buf, size);
}

Buffer Buffer::from(const String& str, const String& encoding) {
  return Buffer(str, encoding);
}

// Buffer Buffer::from(const byte* buf) {
//   return buf;
// }

Buffer Buffer::alloc(int size) {
  return size;
}

const byte* Buffer::buffer() const {
  return _buffer;
}

int Buffer::length() const {
  if (!_buffer)
    return 0;
  return _length;
}

String Buffer::toString() const {
  if (!_buffer) {
    return "";
  }
  char* str = new char[_length + 1]{0};
  for (int i = 0; i < _length; i++) {
    str[i] = _buffer[i];
  }
  String res = str;
  delete[] str;
  str = nullptr;
  return res;
}

String Buffer::toString(const String& encoding) const {
  if (!_buffer) {
    return "";
  }

  if (encoding == "utf8") {
    return this->toString();
  }

  if (encoding == "base64") {
    char* enc = b64_encode(_buffer, _length);
    String res = enc;
    free(enc);
    enc = nullptr;
    return res;
  }

  if (encoding == "hex") {
    String res = "";
    for (int i = 0; i < _length; i++) {
			res += _map[_buffer[i] >> 4];
			res += _map[_buffer[i] & 0x0f];
    }
    return res;
  }

  return "";
}

std::vector<byte> Buffer::toVector() const {
  std::vector<byte> buf;
  for (int i = 0; i < _length; i++) {
    buf.push_back(_buffer[i]);
  }
  return buf;
}

const byte& Buffer::operator[](int index) const {
  if (!_buffer) throw std::exception("Bad buffer.");
  if (index < 0) return _buffer[_length + (index % _length)];
  return _buffer[index % _length];
}

byte& Buffer::operator[](int index) {
  if (!_buffer) throw std::exception("Bad buffer.");
  if (index < 0) return _buffer[_length + (index % _length)];
  return _buffer[index % _length];
}

Buffer Buffer::concat(const Buffer& buf) const {
  int len = _length + buf._length;
  Buffer res = Buffer::alloc(len);
  for (int i = 0; i < _length; i++) {
    res[i] = _buffer[i];
  }
  for (int i = 0; i < buf._length; i++) {
    res[i + _length] = buf[i];
  }
  return res;
}

Buffer Buffer::slice(int start) const {
  return this->slice(start, _length);
}

Buffer Buffer::slice(int start, int end) const {
  if (!_buffer) throw std::exception("Bad buffer.");
  end--;
  start = start < 0 ? (_length + (start % _length)) : start % _length;
  end = end < 0 ? (_length + (end % _length)) : end % _length;
  if (end < start) {
    int tmp = end;
    end = start;
    start = tmp;
  }

  int len = end - start + 1;

  if (len <= 0) return Buffer();

  Buffer res = Buffer::alloc(len);
  for (int i = 0; i < len; i++) {
    res[i] = (*this)[start + i];
  }
  return res;
}
