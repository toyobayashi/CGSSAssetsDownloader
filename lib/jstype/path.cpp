#include "path.h"

#ifdef _WIN32
String path::_sep = "\\";
#elif
String path::_sep = "/";
#endif

bool path::_isPathSeparator(int code) {
  return code == 47 || code == 92;
}
bool path::_isPosixPathSeparator(int code) {
  return code == 47;
}
bool path::_isWindowsDeviceRoot(int code) {
  return (code >= 65 && code <= 90) || (code >= 97 && code <= 122);
}

String path::_normalizeString(const String& path, bool allowAboveRoot, const String& separator, bool (*isPathSeparator)(int)) {
  String res = "";
  int lastSegmentLength = 0;
  int lastSlash = -1;
  int dots = 0;
  int code;
  for (int i = 0; i <= path.length(); ++i) {
    if (i < path.length())
      code = path.charCodeAt(i);
    else if (isPathSeparator(code))
      break;
    else
      code = 47;

    if (isPathSeparator(code)) {
      if (lastSlash == i - 1 || dots == 1) {
        // NOOP
      } else if (lastSlash != i - 1 && dots == 2) {
        if (res.length() < 2 || lastSegmentLength != 2 ||
          res.charCodeAt(res.length() - 1) != 46 ||
          res.charCodeAt(res.length() - 2) != 46) {
          if (res.length() > 2) {
            const int lastSlashIndex = res.lastIndexOf(separator);
            if (lastSlashIndex == -1) {
              res = "";
              lastSegmentLength = 0;
            } else {
              res = res.slice(0, lastSlashIndex);
              lastSegmentLength = res.length() - 1 - res.lastIndexOf(separator);
            }
            lastSlash = i;
            dots = 0;
            continue;
          } else if (res.length() == 2 || res.length() == 1) {
            res = "";
            lastSegmentLength = 0;
            lastSlash = i;
            dots = 0;
            continue;
          }
        }
        if (allowAboveRoot) {
          if (res.length() > 0)
            res += (separator + "..");
          else
            res = "..";
          lastSegmentLength = 2;
        }
      } else {
        if (res.length() > 0)
          res += separator + path.slice(lastSlash + 1, i);
        else
          res = path.slice(lastSlash + 1, i);
        lastSegmentLength = i - lastSlash - 1;
      }
      lastSlash = i;
      dots = 0;
    } else if (code == 46 && dots != -1) {
      ++dots;
    } else {
      dots = -1;
    }
  }
  return res;
}

String path::normalize(const String& path) {
#ifdef _WIN32
  const int len = path.length();
  if (len == 0)
    return '.';
  int rootEnd = 0;
  String* device = nullptr;
  bool isAbsolute = false;
  const int code = path.charCodeAt(0);

  if (len > 1) {
    if (_isPathSeparator(code)) {
      isAbsolute = true;

      if (_isPathSeparator(path.charCodeAt(1))) {
        int j = 2;
        int last = j;
        for (; j < len; ++j) {
          if (_isPathSeparator(path.charCodeAt(j)))
            break;
        }
        if (j < len && j != last) {
          String firstPart = path.slice(last, j);
          last = j;
          for (; j < len; ++j) {
            if (!_isPathSeparator(path.charCodeAt(j)))
              break;
          }
          if (j < len && j != last) {
            last = j;
            for (; j < len; ++j) {
              if (_isPathSeparator(path.charCodeAt(j)))
                break;
            }
            if (j == len) {

              return String("\\\\") + firstPart + '\\' + path.slice(last) + '\\';
            } else if (j != last) {
              String tmp = String("\\\\") + firstPart + '\\' + path.slice(last, j);
              device = &tmp;
              rootEnd = j;
            }
          }
        }
      } else {
        rootEnd = 1;
      }
    } else if (_isWindowsDeviceRoot(code)) {

      if (path.charCodeAt(1) == 58) {
        String tmp = path.slice(0, 2);
        device = device = &tmp;
        rootEnd = 2;
        if (len > 2) {
          if (_isPathSeparator(path.charCodeAt(2))) {
            isAbsolute = true;
            rootEnd = 3;
          }
        }
      }
    }
  } else if (_isPathSeparator(code)) {
    return '\\';
  }

  String tail;
  if (rootEnd < len) {
    tail = _normalizeString(path.slice(rootEnd), !isAbsolute, '\\', path::_isPathSeparator);
  } else {
    tail = "";
  }
  if (tail.length() == 0 && !isAbsolute)
    tail = '.';
  if (tail.length() > 0 && _isPathSeparator(path.charCodeAt(len - 1)))
    tail += '\\';
  if (device == nullptr) {
    if (isAbsolute) {
      if (tail.length() > 0)
        return String('\\') + tail;
      else
        return '\\';
    } else if (tail.length() > 0) {
      return tail;
    } else {
      return "";
    }
  } else if (isAbsolute) {
    if (tail.length() > 0)
      return *device + '\\' + tail;
    else
      return *device + '\\';
  } else if (tail.length() > 0) {
    return *device + tail;
  } else {
    return *device;
  }
#elif
  if (path.length() == 0)
    return '.';

  const bool isAbsolute = (path.charCodeAt(0) == 47);
  const bool trailingSeparator =
    (path.charCodeAt(path.length() - 1) == 47);

  String newpath = _normalizeString(path, !isAbsolute, '/', _isPosixPathSeparator);

  if (newpath.length() == 0 && !isAbsolute)
    newpath = '.';
  if (newpath.length() > 0 && trailingSeparator)
    newpath += '/';

  if (isAbsolute)
    return String('/') + newpath;
  return newpath;
#endif
}

bool path::isAbsolute(const String& path) {
#ifdef _WIN32
  const int len = path.length();
  if (len == 0)
    return false;

  const int code = path.charCodeAt(0);
  if (_isPathSeparator(code)) {
    return true;
  } else if (_isWindowsDeviceRoot(code)) {

    if (len > 2 && path.charCodeAt(1) == 58) {
      if (_isPathSeparator(path.charCodeAt(2)))
        return true;
    }
  }
  return false;
#else
  return path.length() > 0 && path.charCodeAt(0) == 47;
#endif
}

String path::join(const String& arg1) {
  return join(arg1, "");
}

String path::join(const String& arg1, const String& arg2) {
#ifdef _WIN32
  String joined = "";
  String firstPart = "";

  if (arg1.length() > 0) {
    if (joined == "")
      joined = firstPart = arg1;
    else
      joined += (String('\\') + arg1);
  }

  if (arg2.length() > 0) {
    if (joined == "")
      joined = firstPart = arg2;
    else
      joined += (String('\\') + arg2);
  }

  if (joined == "")
    return '.';

  bool needsReplace = true;
  int slashCount = 0;
  if (_isPathSeparator(firstPart.charCodeAt(0))) {
    ++slashCount;
    const int firstLen = firstPart.length();
    if (firstLen > 1) {
      if (_isPathSeparator(firstPart.charCodeAt(1))) {
        ++slashCount;
        if (firstLen > 2) {
          if (_isPathSeparator(firstPart.charCodeAt(2)))
            ++slashCount;
          else {
            needsReplace = false;
          }
        }
      }
    }
  }
  if (needsReplace) {
    for (; slashCount < joined.length(); ++slashCount) {
      if (!_isPathSeparator(joined.charCodeAt(slashCount)))
        break;
    }

    if (slashCount >= 2)
      joined = (String('\\') + joined.slice(slashCount));
  }

  return normalize(joined);
#else
  if (arguments.length() == 0)
    return '.';
  String joined = "";

  if (arg1.length() > 0) {
    if (joined == "")
      joined = arg1;
    else
      joined += (String('/') + arg1);
  }

  if (arg2.length() > 0) {
    if (joined == "")
      joined = arg2;
    else
      joined += (String('/') + arg2);
  }

  if (joined == "")
    return '.';
  return normalize(joined);
#endif
}

String path::dirname(const String& path) {
#ifdef _WIN32
  const int len = path.length();
  if (len == 0)
    return '.';
  int rootEnd = -1;
  int end = -1;
  bool matchedSlash = true;
  int offset = 0;
  int code = path.charCodeAt(0);

  if (len > 1) {
    if (_isPathSeparator(code)) {

      rootEnd = offset = 1;

      if (_isPathSeparator(path.charCodeAt(1))) {
        int j = 2;
        int last = j;
        for (; j < len; ++j) {
          if (_isPathSeparator(path.charCodeAt(j)))
            break;
        }
        if (j < len && j != last) {
          last = j;
          for (; j < len; ++j) {
            if (!_isPathSeparator(path.charCodeAt(j)))
              break;
          }
          if (j < len && j != last) {
            last = j;
            for (; j < len; ++j) {
              if (_isPathSeparator(path.charCodeAt(j)))
                break;
            }
            if (j == len) {
              return path;
            }
            if (j != last) {
              rootEnd = offset = j + 1;
            }
          }
        }
      }
    } else if (_isWindowsDeviceRoot(code)) {
      if (path.charCodeAt(1) == 58) {
        rootEnd = offset = 2;
        if (len > 2) {
          if (_isPathSeparator(path.charCodeAt(2)))
            rootEnd = offset = 3;
        }
      }
    }
  } else if (_isPathSeparator(code)) {
    return path;
  }

  for (int i = len - 1; i >= offset; --i) {
    if (_isPathSeparator(path.charCodeAt(i))) {
      if (!matchedSlash) {
        end = i;
        break;
      }
    } else {
      matchedSlash = false;
    }
  }

  if (end == -1) {
    if (rootEnd == -1)
      return '.';
    else
      end = rootEnd;
  }
  return path.slice(0, end);
#elif
  if (path.length() == 0)
    return '.';
  const bool hasRoot = (path.charCodeAt(0) == 47);
  int end = -1;
  bool matchedSlash = true;
  for (int i = path.length() - 1; i >= 1; --i) {
    if (path.charCodeAt(i) == 47) {
      if (!matchedSlash) {
        end = i;
        break;
      }
    } else {
      matchedSlash = false;
    }
  }

  if (end == -1)
    return hasRoot ? '/' : '.';
  if (hasRoot && end == 1)
    return '//';
  return path.slice(0, end);
#endif
}
