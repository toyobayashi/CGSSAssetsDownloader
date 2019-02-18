#ifndef __JS_STRING_H__
#define __JS_STRING_H__

#ifdef _WIN32
#include <Windows.h>
#endif

#include <cstring>
#include <regex>
#include <string>
template <typename T> class Array;

class String {
 public:
  virtual ~String();
  String();
  String(const char);
  String(const char*);
  String(const std::string&);
  String(const String&);

  const String& operator=(const String&);

  bool operator==(const String&) const;

  bool operator!=(const String&) const;

  String operator+(const String&) const;

  String& operator+=(const String&);

  String operator[](int index) const;

  const char* toCString() const;
  std::string toCppString() const;
  int length() const;
  int byteLength() const;

  String charAt(int index = 0) const;
  int charCodeAt(int index = 0) const;
  String substring(int) const;
  String substring(int, int) const;
  bool endsWith(const String&) const;
  bool endsWith(const String&, int) const;
  int indexOf(const String&) const;
  int indexOf(const String&, int) const;
  int lastIndexOf(const String&) const;
  int lastIndexOf(const String&, int) const;
  bool includes(const String&) const;
  bool includes(const String&, int) const;
  String replace(const std::regex&, const String&) const;
  String replace(const String&, const String&) const;
  String slice(int) const;
  String slice(int, int) const;
  String repeat(int) const;
  String toLowerCase() const;
  String toUpperCase() const;
  String trim() const;
  String trimRight() const;
  String trimLeft() const;
  Array<String> split() const;
  Array<String> split(const String& separator, int limit = -1) const;
  std::wstring toWCppString() const;

  template <typename T>
  String concat(const T& str) const {
    return (*this + str);
  }

  template <typename T, typename... Arg>
  String concat(const T& str, Arg... args) const {
    return concat(str).concat(args...);
  }

  template <typename T>
  static String fromCharCode(T num) {
    int wc = static_cast<int>(num);
    if (wc <= 0x7f) {
      char res[2] = {(char)wc, '\0'};
      return res;
    }
    if (wc <= 0x7ff) {
      char res[3] = {0};
      res[0] = (char)(0xc0 | (wc >> 6));
      res[1] = (char)(0x80 | (wc & 0x3f));
      return res;
    }
    if (wc <= 0xffff) {
      char res[4] = {0};
      res[0] = (char)(0xe0 | (wc >> 12));
      res[1] = (char)(0x80 | ((wc >> 6) & 0x3f));
      res[2] = (char)(0x80 | (wc & 0x3f));
      return res;
    }
    return "";
  }

  template <typename T, typename... Arg>
  static String fromCharCode(T num, Arg... args) {
    String res = fromCharCode(num);
    res += fromCharCode(args...);
    return res;
  }

 private:
  char* _value;

  static int _getByteLengthOfWideChar(const wchar_t*);
  static char* _wideCharToUTF8(const wchar_t*, int);
  static char* _wideCharToUTF8(const wchar_t*);
};

#endif  // __JS_STRING_H__
