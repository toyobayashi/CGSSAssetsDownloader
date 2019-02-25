#include "JSString.h"
#include "JSArray.hpp"
#include <cstring>

int String::_getByteLengthOfWideChar(const wchar_t* wStr) {
#ifdef _WIN32
  return WideCharToMultiByte(CP_UTF8, 0, wStr, -1, nullptr, 0, nullptr,
    nullptr) -
    1;
#else
  int result = 0;
  while (*wStr != L'\0') {
    wchar_t wc = *wStr;

    if (wc <= 0x7f) {
      result += 1;
    } else if (wc <= 0x7ff) {
      result += 2;
    } else if (wc <= 0xffff) {
      result += 3;
    } else {
      result += 0;
    }

    wStr++;
  }
  return result;
#endif
}

char* String::_wideCharToUTF8(const wchar_t* wStr) {
  int utf8ByteLength = String::_getByteLengthOfWideChar(wStr);
  return String::_wideCharToUTF8(wStr, utf8ByteLength);
}

char* String::_wideCharToUTF8(const wchar_t* wStr, int utf8ByteLength) {
  char* utf8 = new (std::nothrow) char[utf8ByteLength + 1];
  if (utf8 == nullptr) {
    return new char[1]{ '\0' };
  }

#ifdef _WIN32
  WideCharToMultiByte(CP_UTF8, 0, wStr, -1, utf8, utf8ByteLength + 1, nullptr,
    nullptr);
#else

  int index = 0;

  while (*wStr != L'\0') {
    wchar_t wc = *wStr;
    if (wc <= 0x7f) {
      utf8[index] = (char)wc;
      index++;
    } else if (wc <= 0x7ff) {
      utf8[index] = (char)(0xc0 | (wc >> 6));
      utf8[index + 1] = (char)(0x80 | (wc & 0x3f));
      index += 2;
    } else if (wc <= 0xffff) {
      utf8[index] = (char)(0xe0 | (wc >> 12));
      utf8[index + 1] = (char)(0x80 | ((wc >> 6) & 0x3f));
      utf8[index + 2] = (char)(0x80 | (wc & 0x3f));
      index += 3;
    }
    wStr++;
  }
  utf8[index] = '\0';
#endif
  return utf8;
}

String::String() {
  _value = "";
}

String::String(const char c) {
  const char tmp[2] = { c, '\0' };
  _value = tmp;
}

String::String(const char* cStr) {
  _value = cStr;
}

String::String(const std::string& str) {
  _value = str;
}

String::String(const String& str) {
  _value = str._value;
}

const String& String::operator=(const char c) {
  const char tmp[2] = { c, '\0' };
  _value = tmp;
  return *this;
}
const String& String::operator=(const char* cstr) {
  _value = cstr;
  return *this;
}
const String& String::operator=(const std::string& str) {
  _value = str;
  return *this;
}
const String& String::operator=(const String& str) {
  if (this == &str) {
    return *this;
  }
  _value = str._value;
  return *this;
}

bool String::operator==(const char c) const {
  const char tmp[2] = { c, '\0' };
  return _value == tmp;
}
bool String::operator==(const char* cstr) const {
  return _value == cstr;
}
bool String::operator==(const std::string& str) const {
  return _value == str;
}
bool String::operator==(const String& str) const {
  return _value == str._value;
}

bool String::operator!=(const char c) const {
  const char tmp[2] = { c, '\0' };
  return _value != tmp;
}
bool String::operator!=(const char* cstr) const {
  return _value != cstr;
}
bool String::operator!=(const std::string& str) const {
  return _value != str;
}
bool String::operator!=(const String& str) const {
  return _value != str._value;
}

String String::operator+(const char c) const {
  const char tmp[2] = { c, '\0' };
  return _value + tmp;
}
String String::operator+(const char* cstr) const {
  return _value + cstr;
}
String String::operator+(const std::string& str) const {
  return _value + str;
}
String String::operator+(const String& str) const {
  return _value + str._value;
}

String& String::operator+=(const String& str) {
  _value += str._value;
  return *this;
}
String& String::operator+=(const char c) {
  char tmp[2] = { c, '\0'};
  _value += tmp;
  return *this;
}
String& String::operator+=(const char* str) {
  _value += str;
  return *this;
}
String& String::operator+=(const std::string& str) {
  _value += str;
  return *this;
}

String String::operator[](int index) const {
  int l = this->length();
  if (l == 0)
    return _value[0];
  if (index >= l) {
    index = index % l;
  } else if (index < 0) {
    index = l + ((index % l) == 0 ? -l : (index % l));
  }

  return charAt(index);
}

int String::charCodeAt(int index) const {
  int length = this->length();
  if (index < 0) {
    return -1;
  }

  if (index > length - 1) {
    return -1;
  }

  String c = this->charAt(index);
#ifdef _WIN32
  wchar_t wc[2] = { 0 };
  MultiByteToWideChar(CP_UTF8, 0, c._value.c_str(), -1, wc, 2);
  return wc[0];
#else
  int byteLength = c.byteLength();
  if (byteLength == 2) {
    return c._value[0];
  }

  if (byteLength == 3) {
    return (((0x1f & c._value[0]) << 6) | (0x7f & c._value[1]));
  }

  if (byteLength == 4) {
    return ((0x0f & c._value[0]) << 12) | ((0x7f & c._value[1]) << 6) | (0x7f & c._value[2]));
  }
  return -1;
#endif
}

String String::charAt(int index) const {
  int length = this->length();
  if (index < 0) {
    const char res[1] = { '\0' };
    return res;
  }

  if (index > length - 1) {
    const char res[1] = { '\0' };
    return res;
  }

  const char* p = _value.c_str();
  int result = 0;
  while (*p != '\0' && result < index) {
    if ((((uint8_t)*p) >> 7) == 0) {
      result++;
      p++;
    } else if ((((uint8_t)*p) >> 5) == 6) {
      result++;
      p += 2;
    } else if ((((uint8_t)*p) >> 4) == 14) {
      result++;
      p += 3;
    } else {
      p++;
    }
  }

  if ((((uint8_t)*p) >> 7) == 0) {
    const char res[2] = { *p, '\0' };
    return res;
  } else if ((((uint8_t)*p) >> 5) == 6) {
    const char res[3] = { *p, *(p + 1), '\0' };
    return res;
  } else if ((((uint8_t)*p) >> 4) == 14) {
    const char res[4] = { *p, *(p + 1), *(p + 2), '\0' };
    return res;
  } else {
    const char res[1] = { '\0' };
    return res;
  }
}

String::~String() {}

const char* String::toCString() const {
  return _value.c_str();
}

std::string String::toCppString() const {
  return _value;
}

int String::length() const {
#ifdef _WIN32
  return MultiByteToWideChar(CP_UTF8, 0, _value.c_str(), -1, nullptr, 0) - 1;
#else
  char* p = _value.c_str();
  int result = 0;
  while (*p != '\0') {
    if ((((uint8_t)*p) >> 7) == 0) {
      result++;
      p++;
    } else if ((((uint8_t)*p) >> 5) == 6) {
      result++;
      p += 2;
    } else if ((((uint8_t)*p) >> 4) == 14) {
      result++;
      p += 3;
    } else {
      p++;
    }
  }
  return result;
#endif
}

int String::byteLength() const {
  return (int)_value.size();
}

String String::substring(int indexStart) const {
  int l = this->length();
  if (l == 0)
    return _value[0];
  if (indexStart >= l) {
    indexStart = l;
  } else if (indexStart < 0) {
    indexStart = 0;
  }

  const char* p = _value.c_str();
  int result = 0;
  while (*p != '\0' && result < indexStart) {
    if ((((uint8_t)*p) >> 7) == 0) {
      result++;
      p++;
    } else if ((((uint8_t)*p) >> 5) == 6) {
      result++;
      p += 2;
    } else if ((((uint8_t)*p) >> 4) == 14) {
      result++;
      p += 3;
    } else {
      p++;
    }
  }

  return p;
}

String String::substring(int indexStart, int indexEnd) const {
  int l = this->length();
  if (l == 0)
    return _value[0];
  if (indexStart >= l) {
    indexStart = l;
  } else if (indexStart < 0) {
    indexStart = 0;
  }

  if (indexEnd >= l) {
    indexEnd = l;
  } else if (indexEnd < 0) {
    indexEnd = 0;
  }

  if (indexStart == indexEnd) return "";

  if (indexEnd < indexStart) {
    int tmp = indexStart;
    indexStart = indexEnd;
    indexEnd = tmp;
  }

  String res = "";

  for (int i = indexStart; i < indexEnd; i++) {
    res += (*this)[i];
  }

  return res;
}

bool String::endsWith(const String& searchString) const {
  int position = length();
  return substring(position - searchString.length(), position) == searchString;
}

bool String::endsWith(const String& searchString, int position) const {
  return substring(position - searchString.length(), position) == searchString;
}

int String::indexOf(const String& searchValue) const {
  return this->indexOf(searchValue, 0);
}

int String::indexOf(const String& searchValue, int fromIndex) const {
  int thisLength = this->length();
  if (searchValue == "") {
    return fromIndex <= 0 ? 0 : (fromIndex <= thisLength ? fromIndex : thisLength);
  }


  if (fromIndex >= thisLength) {
    return -1;
  }

  if (fromIndex < 0) {
    fromIndex = 0;
  }

  int len = searchValue.length();
  for (int i = fromIndex; i < this->length(); i++) {
    if (searchValue == this->substring(i, i + len)) {
      return i;
    }
  }
  return -1;
}

int String::lastIndexOf(const String& searchValue) const {
  return this->lastIndexOf(searchValue, this->length());
}

int String::lastIndexOf(const String& searchValue, int fromIndex) const {
  int thisLength = this->length();
  if (fromIndex < 0) {
    fromIndex = 0;
  } else if (fromIndex > thisLength) {
    fromIndex = thisLength;
  }

  int len = searchValue.length();
  for (int i = fromIndex - 1; i >= 0; i--) {
    if (searchValue == this->substring(i, i + len)) {
      return i;
    }
  }
  return -1;
}

bool String::includes(const String& searchString) const {
  return this->includes(searchString, 0);
}

bool String::includes(const String& searchString, int position) const {
  if (position + searchString.length() > this->length()) {
    return false;
  } else {
    return -1 != this->indexOf(searchString, position);
  }
}

String String::replace(const std::regex& re, const String& replace) const {
  std::string str = this->toCppString();
  return std::regex_replace(str, re, replace.toCppString());
}

String String::replace(const String& searchString, const String& replace) const {
  std::string str = this->toCppString();
  int i = this->indexOf(searchString);
  if (i == -1) {
    return str;
  }

  return str.replace(i, searchString.byteLength(), replace.toCppString());
}

String String::slice(int start) const {
  return this->slice(start, length());
}

String String::slice(int start, int end) const {
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

  if (len <= 0) return "";

  return substring(start, end + 1);
}

String String::repeat(int n) const {
  if (n <= 0) {
    return "";
  }

  String res = "";
  for (int i = 0; i < n; i++) {
    res += (*this);
  }

  return res;
}

String String::toLowerCase() const {
  int bl = byteLength();
  char* res = new char[bl + 1]{ 0 };
  for (int i = 0; i < bl; i++) {
    if (_value[i] >= 65 && _value[i] <= 90) {
      res[i] = _value[i] + 32;
    } else {
      res[i] = _value[i];
    }
  }
  String str = res;
  delete[] res;
  res = nullptr;
  return str;
}

String String::toUpperCase() const {
  int bl = byteLength();
  char* res = new char[bl + 1]{ 0 };
  for (int i = 0; i < bl; i++) {
    if (_value[i] >= 97 && _value[i] <= 122) {
      res[i] = _value[i] - 32;
    } else {
      res[i] = _value[i];
    }
  }
  String str = res;
  delete[] res;
  res = nullptr;
  return str;
}

String String::trim() const {
  return replace(std::regex("^[\\s\\xA0]+|[\\s\\xA0]+$"), "");
}

String String::trimRight() const {
  return replace(std::regex("[\\s\\xA0]+$"), "");
}

String String::trimLeft() const {
  return replace(std::regex("^[\\s\\xA0]+"), "");
}

std::wstring String::toWCppString() const {
#ifdef _WIN32
  int wLength = MultiByteToWideChar(CP_UTF8, 0, _value.c_str(), -1, nullptr, 0);
  wchar_t* buf = new wchar_t[wLength]{0};
  MultiByteToWideChar(CP_UTF8, 0, _value.c_str(), -1, buf, wLength);
  std::wstring res = buf;
  delete[] buf;
  buf = nullptr;
  return res;
#else
  int wcharLength = this->length();
  wchar_t* buf = new wchar_t[wcharLength + 1]{0};

  char* p = _value.c_str();
  int index = 0;
  while (*p != '\0') {
    if ((((uint8_t)*p) >> 7) == 0) {
      buf[index] = (wchar_t)(*p);
      index++;
      p++;
    } else if ((((uint8_t)*p) >> 5) == 6) {
      buf[index] = (wchar_t)((((*p) & 0x1f) << 6) | ((*(p + 1)) & 0x3f));
      index++;
      p += 2;
    } else if ((((uint8_t)*p) >> 4) == 14) {
      buf[index] = (wchar_t)((((*p) & 0x0f) << 12) | (((*(p + 1)) & 0x3f) << 6) | ((*(p + 2)) & 0x3f));
      index++;
      p += 3;
    } else {
      p++;
    }
  }

  std::wstring res = buf;
  delete[] buf;
  buf = nullptr;
  return res;
#endif
}

Array<String> String::split() const {
  return { *this };
}

Array<String> String::split(const String& separator, int limit) const {
  std::string copy = this->toCppString();
  char* copyBuf = new char[copy.size() + 1]{0};
  strcpy(copyBuf, copy.c_str());

  char* tokenPtr = strtok(copyBuf, separator.toCString());
  Array<String> res;
  while (tokenPtr != NULL && (limit == -1 ? true : res.length() < limit)) {
    res.push(tokenPtr);
    tokenPtr = strtok(NULL, separator.toCString());
  }
  delete[] copyBuf;
  return res;
}
