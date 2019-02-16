#include "./JSString.h"
#include <iostream>

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
    return new char[1]{'\0'};
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
  _value = new char[1]{'\0'};
}

String::String(const char c) {
  _value = new char[2]{c, '\0'};
}

String::String(const char* cStr) {
  _value = new char[strlen(cStr) + 1]{0};
  strcpy(_value, cStr);
}

String::String(const std::string& str) {
  _value = new char[str.length() + 1]{0};
  strcpy(_value, str.c_str());
}

String::String(const String& str) {
  _value = new char[strlen(str._value) + 1]{0};
  strcpy(_value, str._value);
}

const String& String::operator=(const String& str) {
  if (this == &str) {
    return *this;
  }

  char* tmp = new (std::nothrow) char[strlen(str._value) + 1];
  if (tmp == nullptr) {
    return *this;
  }

  strcpy(tmp, str._value);

  delete[] _value;
  _value = tmp;
  tmp = nullptr;

  return *this;
}

bool String::operator==(const String& str) const {
  return strcmp(_value, str._value) == 0;
}

bool String::operator!=(const String& str) const {
  return !(*this == str);
}

String String::operator+(const String& str) const {
  int totalLength =
      static_cast<int>(strlen(_value)) + static_cast<int>(strlen(str._value));
  char* tmp = new char[totalLength + 1]{0};

  strcpy(tmp, _value);
  strcat(tmp, str._value);

  String res = tmp;
  delete[] tmp;
  tmp = nullptr;
  return res;
}

String& String::operator+=(const String& str) {
  int totalLength = this->byteLength() + static_cast<int>(strlen(str._value));
  char* tmp = new char[totalLength + 1]{0};

  strcpy(tmp, _value);
  strcat(tmp, str._value);

  delete[] _value;
  _value = tmp;
  tmp = nullptr;

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
  wchar_t wc[2] = {0};
  MultiByteToWideChar(CP_UTF8, 0, c._value, -1, wc, 2);
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
    const char res[1] = {'\0'};
    return res;
  }

  if (index > length - 1) {
    const char res[1] = {'\0'};
    return res;
  }

  char* p = _value;
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
    const char res[2] = {*p, '\0'};
    return res;
  } else if ((((uint8_t)*p) >> 5) == 6) {
    const char res[3] = {*p, *(p + 1), '\0'};
    return res;
  } else if ((((uint8_t)*p) >> 4) == 14) {
    const char res[4] = {*p, *(p + 1), *(p + 2), '\0'};
    return res;
  } else {
    const char res[1] = {'\0'};
    return res;
  }
}

String::~String() {
  delete[] _value;
  _value = nullptr;
}

const char* String::toCString() const {
  return _value;
}

std::string String::toCppString() const {
  return _value;
}

int String::length() const {
#ifdef _WIN32
  return MultiByteToWideChar(CP_UTF8, 0, _value, -1, nullptr, 0) - 1;
#else
  char* p = _value;
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
  return static_cast<int>(strlen(_value));
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

  char* p = _value;
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
  char* res = new char[bl + 1]{0};
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
  char* res = new char[bl + 1]{0};
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
  int l = length();
  String res = "";
  for (int i = 0; i < l; i++) {
    String c = (*this)[i];
    if (c != " ") {
      res += c;
    }
  }
  return res;
}

String String::trimRight() const {
  int l = length();
  String res = "";
  bool flag = true;
  for (int i = l - 1; i >= 0; i--) {
    String c = (*this)[i];
    if (c != " ") {
      flag = false;
      res = c + res;
    } else {
      if (!flag) {
        res = c + res;
      }
    }
  }
  return res;
}

String String::trimLeft() const {
  int l = length();
  String res = "";
  bool flag = true;
  for (int i = 0; i < l; i++) {
    String c = (*this)[i];
    if (c != " ") {
      flag = false;
      res += c;
    } else {
      if (!flag) {
        res += c;
      }
    }
  }
  return res;
}
