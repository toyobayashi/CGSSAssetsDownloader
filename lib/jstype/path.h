#ifndef __PATH_HPP__
#define __PATH_HPP__

#include "JSString.h"

class path {
public:
  virtual ~path() {};
  static String sep() { return _sep; }
  static String dirname(const String&);
  static String normalize(const String&);
  static bool isAbsolute(const String&);

  static String join(const String&);
  static String join(const String&, const String&);

  template <typename... Args>
  static String join(const String& arg1, const String& arg2, Args... args) {
    String tmp = join(arg1, arg2);
    return join(tmp, args...);
  }

private:
  path() {};
  static String _sep;
  static bool _isPathSeparator(int);
  static bool _isPosixPathSeparator(int);
  static bool _isWindowsDeviceRoot(int);
  static String _normalizeString(const String&, bool, const String&, bool (*)(int));
};

#endif
