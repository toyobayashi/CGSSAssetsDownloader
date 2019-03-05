#include "path.h"
#include "jsprocess.h"
#include "JSArray.hpp"

#ifdef _WIN32
#include "Windows.h"
String path::_sep = "\\";
String path::_delimiter = ";";
#elif
String path::_sep = "/";
String path::_delimiter = ":";
#endif

bool path::_isPathSeparator(int code) {
  return code == CHAR_FORWARD_SLASH || code == CHAR_BACKWARD_SLASH;
}
bool path::_isPosixPathSeparator(int code) {
  return code == CHAR_FORWARD_SLASH;
}
bool path::_isWindowsDeviceRoot(int code) {
  return (code >= CHAR_UPPERCASE_A && code <= CHAR_UPPERCASE_Z) || (code >= CHAR_LOWERCASE_A && code <= CHAR_LOWERCASE_Z);
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
      code = CHAR_FORWARD_SLASH;

    if (isPathSeparator(code)) {
      if (lastSlash == i - 1 || dots == 1) {
        // NOOP
      } else if (lastSlash != i - 1 && dots == 2) {
        if (res.length() < 2 || lastSegmentLength != 2 ||
          res.charCodeAt(res.length() - 1) != CHAR_DOT ||
          res.charCodeAt(res.length() - 2) != CHAR_DOT) {
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
    } else if (code == CHAR_DOT && dots != -1) {
      ++dots;
    } else {
      dots = -1;
    }
  }
  return res;
}

String path::_format(const String& sep, const FormatInputPathObject& pathObject) {
  const String& dir = (pathObject.dir == "" ?  pathObject.root : pathObject.dir);
  const String& base = (pathObject.base != "" ? pathObject.base :
    (pathObject.name + pathObject.ext));
  if (dir == "") {
    return base;
  }
  if (dir == pathObject.root) {
    return dir + base;
  }
  return dir + sep + base;
}

String path::format(const FormatInputPathObject& pathObject) {
#ifdef _WIN32
  return _format("\\", pathObject);
#else
  return _format("/", pathObject);
#endif
}

String path::toNamespacedPath(const String& path) {
#ifdef _WIN32
  if (path == "") {
    return "";
  }

  const String resolvedPath = resolve(path);

  if (resolvedPath.length() >= 3) {
    if (resolvedPath.charCodeAt(0) == CHAR_BACKWARD_SLASH) {

      if (resolvedPath.charCodeAt(1) == CHAR_BACKWARD_SLASH) {
        const int code = resolvedPath.charCodeAt(2);
        if (code != CHAR_QUESTION_MARK && code != CHAR_DOT) {
          return String("\\\\?\\UNC\\") + resolvedPath.slice(2);
        }
      }
    } else if (_isWindowsDeviceRoot(resolvedPath.charCodeAt(0))) {

      if (resolvedPath.charCodeAt(1) == CHAR_COLON &&
        resolvedPath.charCodeAt(2) == CHAR_BACKWARD_SLASH) {
        return String("\\\\?\\") + resolvedPath;
      }
    }
  }

  return path;
#else
  return path;
#endif
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

      if (path.charCodeAt(1) == CHAR_COLON) {
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

  const bool isAbsolute = (path.charCodeAt(0) == CHAR_FORWARD_SLASH);
  const bool trailingSeparator =
    (path.charCodeAt(path.length() - 1) == CHAR_FORWARD_SLASH);

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

    if (len > 2 && path.charCodeAt(1) == CHAR_COLON) {
      if (_isPathSeparator(path.charCodeAt(2)))
        return true;
    }
  }
  return false;
#else
  return path.length() > 0 && path.charCodeAt(0) == CHAR_FORWARD_SLASH;
#endif
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

path::~path() {}

String path::delimiter() {
  return _delimiter;
}

String path::sep() {
  return _sep;
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
      if (path.charCodeAt(1) == CHAR_COLON) {
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
  const bool hasRoot = (path.charCodeAt(0) == CHAR_FORWARD_SLASH);
  int end = -1;
  bool matchedSlash = true;
  for (int i = path.length() - 1; i >= 1; --i) {
    if (path.charCodeAt(i) == CHAR_FORWARD_SLASH) {
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

String path::basename(const String& path, const String& ext) {
  int start = 0;
  int end = -1;
  bool matchedSlash = true;
  int i;
#ifdef _WIN32
  if (path.length() >= 2) {
    const int drive = path.charCodeAt(0);
    if (_isWindowsDeviceRoot(drive)) {
      if (path.charCodeAt(1) == CHAR_COLON)
        start = 2;
    }
  }

  if (ext.length() > 0 && ext.length() <= path.length()) {
    if (ext.length() == path.length() && ext == path)
      return "";
    int extIdx = ext.length() - 1;
    int firstNonSlashEnd = -1;
    for (i = path.length() - 1; i >= start; --i) {
      const int code = path.charCodeAt(i);
      if (_isPathSeparator(code)) {
        if (!matchedSlash) {
          start = i + 1;
          break;
        }
      } else {
        if (firstNonSlashEnd == -1) {
          matchedSlash = false;
          firstNonSlashEnd = i + 1;
        }
        if (extIdx >= 0) {
          if (code == ext.charCodeAt(extIdx)) {
            if (--extIdx == -1) {
              end = i;
            }
          } else {
            extIdx = -1;
            end = firstNonSlashEnd;
          }
        }
      }
    }

    if (start == end)
      end = firstNonSlashEnd;
    else if (end == -1)
      end = path.length();
    return path.slice(start, end);
  }
#else
  if (ext.length() > 0 && ext.length() <= path.length()) {
    if (ext.length() == path.length() && ext == path)
      return "";
    int extIdx = ext.length() - 1;
    int firstNonSlashEnd = -1;
    for (i = path.length() - 1; i >= 0; --i) {
      const int code = path.charCodeAt(i);
      if (code == CHAR_FORWARD_SLASH) {
        if (!matchedSlash) {
          start = i + 1;
          break;
        }
      } else {
        if (firstNonSlashEnd == -1) {
          matchedSlash = false;
          firstNonSlashEnd = i + 1;
        }
        if (extIdx >= 0) {
          if (code == ext.charCodeAt(extIdx)) {
            if (--extIdx == -1) {
              end = i;
            }
          } else {
            extIdx = -1;
            end = firstNonSlashEnd;
          }
        }
      }
    }

    if (start == end)
      end = firstNonSlashEnd;
    else if (end == -1)
      end = path.length();
    return path.slice(start, end);
  }
#endif
  return path::basename(path);
}

String path::basename(const String& path) {
  int start = 0;
  int end = -1;
  bool matchedSlash = true;
  int i;
#ifdef _WIN32
  if (path.length() >= 2) {
    const int drive = path.charCodeAt(0);
    if (_isWindowsDeviceRoot(drive)) {
      if (path.charCodeAt(1) == CHAR_COLON)
        start = 2;
    }
  }

  for (i = path.length() - 1; i >= start; --i) {
    if (_isPathSeparator(path.charCodeAt(i))) {
      if (!matchedSlash) {
        start = i + 1;
        break;
      }
    } else if (end == -1) {
      matchedSlash = false;
      end = i + 1;
    }
  }
#else
  for (i = path.length() - 1; i >= 0; --i) {
    if (path.charCodeAt(i) == CHAR_FORWARD_SLASH) {
      if (!matchedSlash) {
        start = i + 1;
        break;
      }
    } else if (end == -1) {
      matchedSlash = false;
      end = i + 1;
    }
  }
#endif
  if (end == -1)
    return "";
  return path.slice(start, end);
}

String path::extname(const String& path) {
#ifdef _WIN32
  int start = 0;
  int startDot = -1;
  int startPart = 0;
  int end = -1;
  bool matchedSlash = true;
  int preDotState = 0;

  if (path.length() >= 2 &&
    path.charCodeAt(1) == CHAR_COLON &&
    _isWindowsDeviceRoot(path.charCodeAt(0))) {
    start = startPart = 2;
  }

  for (int i = path.length() - 1; i >= start; --i) {
    const int code = path.charCodeAt(i);
    if (_isPathSeparator(code)) {
      if (!matchedSlash) {
        startPart = i + 1;
        break;
      }
      continue;
    }
    if (end == -1) {
      matchedSlash = false;
      end = i + 1;
    }
    if (code == CHAR_DOT) {
      if (startDot == -1)
        startDot = i;
      else if (preDotState != 1)
        preDotState = 1;
    } else if (startDot != -1) {
      preDotState = -1;
    }
  }

  if (startDot == -1 ||
    end == -1 ||
    preDotState == 0 ||
    (preDotState == 1 &&
      startDot == end - 1 &&
      startDot == startPart + 1)) {
    return "";
  }
  return path.slice(startDot, end);
#else
  int startDot = -1;
  int startPart = 0;
  int end = -1;
  bool matchedSlash = true;
  int preDotState = 0;
  for (int i = path.length() - 1; i >= 0; --i) {
    const int code = path.charCodeAt(i);
    if (code == CHAR_FORWARD_SLASH) {
      if (!matchedSlash) {
        startPart = i + 1;
        break;
      }
      continue;
    }
    if (end == -1) {
      matchedSlash = false;
      end = i + 1;
    }
    if (code == CHAR_DOT) {
      if (startDot == -1)
        startDot = i;
      else if (preDotState != 1)
        preDotState = 1;
    } else if (startDot != -1) {
      preDotState = -1;
    }
  }

  if (startDot == -1 ||
    end == -1 ||
    preDotState == 0 ||
    (preDotState == 1 &&
      startDot == end - 1 &&
      startDot == startPart + 1)) {
    return "";
  }
  return path.slice(startDot, end);
#endif
}

String path::resolve(const String& arg, const String& arg1) {
  Array<String> arguments = { arg, arg1 };
#ifdef _WIN32
  String resolvedDevice = "";
  String resolvedTail = "";
  bool resolvedAbsolute = false;

  for (int i = arguments.length() - 1; i >= -1; i--) {
    String path = "";
    if (i >= 0) {
      path = arguments[i];
    } else if (resolvedDevice == "") {
      path = process::cwd();
    } else {
      wchar_t env[MAX_PATH + 1];
      if (0 == GetEnvironmentVariableW((String("=") + resolvedDevice).toCppWString().c_str(), env, MAX_PATH + 1)) {
        path = process::cwd();
      } else {
        path = String::fromWideChar(env);
      }

      if (path == "" ||
        path.slice(0, 3).toLowerCase() !=
        resolvedDevice.toLowerCase() + '\\') {
        path = resolvedDevice + '\\';
      }
    }

    if (path.length() == 0) {
      continue;
    }

    int len = path.length();
    int rootEnd = 0;
    String device = "";
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
            const String firstPart = path.slice(last, j);
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
                device = String("\\\\") + firstPart + '\\' + path.slice(last);
                rootEnd = j;
              } else if (j != last) {
                device = String("\\\\") + firstPart + '\\' + path.slice(last, j);
                rootEnd = j;
              }
            }
          }
        } else {
          rootEnd = 1;
        }
      } else if (_isWindowsDeviceRoot(code)) {

        if (path.charCodeAt(1) == CHAR_COLON) {
          device = path.slice(0, 2);
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
      rootEnd = 1;
      isAbsolute = true;
    }

    if (device.length() > 0 &&
      resolvedDevice.length() > 0 &&
      device.toLowerCase() != resolvedDevice.toLowerCase()) {
      continue;
    }

    if (resolvedDevice.length() == 0 && device.length() > 0) {
      resolvedDevice = device;
    }
    if (!resolvedAbsolute) {
      resolvedTail = path.slice(rootEnd) + '\\' + resolvedTail;
      resolvedAbsolute = isAbsolute;
    }

    if (resolvedDevice.length() > 0 && resolvedAbsolute) {
      break;
    }
  }
  resolvedTail = _normalizeString(resolvedTail, !resolvedAbsolute, '\\',
    _isPathSeparator);

  String res = resolvedDevice + (resolvedAbsolute ? String('\\') : String("")) + resolvedTail;

  return res == "" ? "." : res;
#else
  String resolvedPath = "";
  bool resolvedAbsolute = false;

  for (int i = arguments.length() - 1; i >= -1 && !resolvedAbsolute; i--) {
    String path;
    if (i >= 0)
      path = arguments[i];
    else {
      path = process::cwd();
    }

    if (path.length() == 0) {
      continue;
    }

    resolvedPath = path + '/' + resolvedPath;
    resolvedAbsolute = (path.charCodeAt(0) == CHAR_FORWARD_SLASH);
  }

  resolvedPath = _normalizeString(resolvedPath, !resolvedAbsolute, '/',
    _isPosixPathSeparator);

  if (resolvedAbsolute) {
    if (resolvedPath.length() > 0)
      return String('/') + resolvedPath;
    else
      return '/';
  } else if (resolvedPath.length() > 0) {
    return resolvedPath;
  } else {
    return '.';
  }

#endif
}

String path::relative(String from, String to) {
#ifdef _WIN32
  if (from == to)
    return "";

  String fromOrig = resolve(from);
  String toOrig = resolve(to);

  if (fromOrig == toOrig)
    return "";

  from = fromOrig.toLowerCase();
  to = toOrig.toLowerCase();

  if (from == to)
    return "";

  int fromStart = 0;
  for (; fromStart < from.length(); ++fromStart) {
    if (from.charCodeAt(fromStart) != CHAR_BACKWARD_SLASH)
      break;
  }
  int fromEnd = from.length();
  for (; fromEnd - 1 > fromStart; --fromEnd) {
    if (from.charCodeAt(fromEnd - 1) != CHAR_BACKWARD_SLASH)
      break;
  }
  int fromLen = (fromEnd - fromStart);

  int toStart = 0;
  for (; toStart < to.length(); ++toStart) {
    if (to.charCodeAt(toStart) != CHAR_BACKWARD_SLASH)
      break;
  }
  int toEnd = to.length();
  for (; toEnd - 1 > toStart; --toEnd) {
    if (to.charCodeAt(toEnd - 1) != CHAR_BACKWARD_SLASH)
      break;
  }
  int toLen = (toEnd - toStart);

  int length = (fromLen < toLen ? fromLen : toLen);
  int lastCommonSep = -1;
  int i = 0;
  for (; i <= length; ++i) {
    if (i == length) {
      if (toLen > length) {
        if (to.charCodeAt(toStart + i) == CHAR_BACKWARD_SLASH) {
          return toOrig.slice(toStart + i + 1);
        } else if (i == 2) {
          return toOrig.slice(toStart + i);
        }
      }
      if (fromLen > length) {
        if (from.charCodeAt(fromStart + i) == CHAR_BACKWARD_SLASH) {
          lastCommonSep = i;
        } else if (i == 2) {
          lastCommonSep = 3;
        }
      }
      break;
    }
    int fromCode = from.charCodeAt(fromStart + i);
    int toCode = to.charCodeAt(toStart + i);
    if (fromCode != toCode)
      break;
    else if (fromCode == CHAR_BACKWARD_SLASH)
      lastCommonSep = i;
  }

  if (i != length && lastCommonSep == -1) {
    return toOrig;
  }

  String out = "";
  if (lastCommonSep == -1)
    lastCommonSep = 0;

  for (i = fromStart + lastCommonSep + 1; i <= fromEnd; ++i) {
    if (i == fromEnd || from.charCodeAt(i) == CHAR_BACKWARD_SLASH) {
      if (out.length() == 0)
        out += "..";
      else
        out += "\\..";
    }
  }

  if (out.length() > 0)
    return out + toOrig.slice(toStart + lastCommonSep, toEnd);
  else {
    toStart += lastCommonSep;
    if (toOrig.charCodeAt(toStart) == CHAR_BACKWARD_SLASH)
      ++toStart;
    return toOrig.slice(toStart, toEnd);
  }
#else
  if (from == to)
    return "";

  from = resolve(from);
  to = resolve(to);

  if (from == to)
    return "";

  int fromStart = 1;
  for (; fromStart < from.length(); ++fromStart) {
    if (from.charCodeAt(fromStart) != CHAR_FORWARD_SLASH)
      break;
  }
  int fromEnd = from.length();
  int fromLen = (fromEnd - fromStart);

  int toStart = 1;
  for (; toStart < to.length(); ++toStart) {
    if (to.charCodeAt(toStart) != CHAR_FORWARD_SLASH)
      break;
  }
  int toEnd = to.length();
  int toLen = (toEnd - toStart);

  // Compare paths to find the longest common path from root
  int length = (fromLen < toLen ? fromLen : toLen);
  int lastCommonSep = -1;
  int i = 0;
  for (; i <= length; ++i) {
    if (i == length) {
      if (toLen > length) {
        if (to.charCodeAt(toStart + i) == CHAR_FORWARD_SLASH) {
          return to.slice(toStart + i + 1);
        } else if (i == 0) {
          return to.slice(toStart + i);
        }
      } else if (fromLen > length) {
        if (from.charCodeAt(fromStart + i) == CHAR_FORWARD_SLASH) {
          lastCommonSep = i;
        } else if (i == 0) {
          lastCommonSep = 0;
        }
      }
      break;
    }
    int fromCode = from.charCodeAt(fromStart + i);
    int toCode = to.charCodeAt(toStart + i);
    if (fromCode != toCode)
      break;
    else if (fromCode == CHAR_FORWARD_SLASH)
      lastCommonSep = i;
  }

  String out = "";
  for (i = fromStart + lastCommonSep + 1; i <= fromEnd; ++i) {
    if (i == fromEnd || from.charCodeAt(i) == CHAR_FORWARD_SLASH) {
      if (out.length() == 0)
        out += "..";
      else
        out += "/..";
    }
  }

  if (out.length() > 0)
    return out + to.slice(toStart + lastCommonSep);
  else {
    toStart += lastCommonSep;
    if (to.charCodeAt(toStart) == CHAR_FORWARD_SLASH)
      ++toStart;
    return to.slice(toStart);
  }
#endif
}

path::FormatInputPathObject path::parse(const String& path) {
  FormatInputPathObject ret;
  ret.root = "";
  ret.dir = "";
  ret.base = "";
  ret.name = "";
  ret.ext = "";
#ifdef _WIN32
  
  if (path.length() == 0)
    return ret;

  int len = path.length();
  int rootEnd = 0;
  int code = path.charCodeAt(0);

  if (len > 1) {
    if (_isPathSeparator(code)) {

      rootEnd = 1;
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
              rootEnd = j;
            } else if (j != last) {
              rootEnd = j + 1;
            }
          }
        }
      }
    } else if (_isWindowsDeviceRoot(code)) {

      if (path.charCodeAt(1) == CHAR_COLON) {
        rootEnd = 2;
        if (len > 2) {
          if (_isPathSeparator(path.charCodeAt(2))) {
            if (len == 3) {
              ret.root = ret.dir = path;
              return ret;
            }
            rootEnd = 3;
          }
        } else {
          ret.root = ret.dir = path;
          return ret;
        }
      }
    }
  } else if (_isPathSeparator(code)) {
    ret.root = ret.dir = path;
    return ret;
  }

  if (rootEnd > 0)
    ret.root = path.slice(0, rootEnd);

  int startDot = -1;
  int startPart = rootEnd;
  int end = -1;
  bool matchedSlash = true;
  int i = path.length() - 1;

  int preDotState = 0;

  for (; i >= rootEnd; --i) {
    code = path.charCodeAt(i);
    if (_isPathSeparator(code)) {
      if (!matchedSlash) {
        startPart = i + 1;
        break;
      }
      continue;
    }
    if (end == -1) {
      matchedSlash = false;
      end = i + 1;
    }
    if (code == CHAR_DOT) {
      if (startDot == -1)
        startDot = i;
      else if (preDotState != 1)
        preDotState = 1;
    } else if (startDot != -1) {
      preDotState = -1;
    }
  }

  if (startDot == -1 ||
    end == -1 ||
    preDotState == 0 ||
    (preDotState == 1 &&
      startDot == end - 1 &&
      startDot == startPart + 1)) {
    if (end != -1) {
      ret.base = ret.name = path.slice(startPart, end);
    }
  } else {
    ret.name = path.slice(startPart, startDot);
    ret.base = path.slice(startPart, end);
    ret.ext = path.slice(startDot, end);
  }

  if (startPart > 0 && startPart != rootEnd)
    ret.dir = path.slice(0, startPart - 1);
  else
    ret.dir = ret.root;

  return ret;
#else
  if (path.length() == 0)
    return ret;
  bool isAbsolute = (path.charCodeAt(0) == CHAR_FORWARD_SLASH);
  int start;
  if (isAbsolute) {
    ret.root = '/';
    start = 1;
  } else {
    start = 0;
  }
  int startDot = -1;
  int startPart = 0;
  int end = -1;
  bool matchedSlash = true;
  int i = path.length() - 1;

  int preDotState = 0;

  for (; i >= start; --i) {
    const int code = path.charCodeAt(i);
    if (code == CHAR_FORWARD_SLASH) {
      if (!matchedSlash) {
        startPart = i + 1;
        break;
      }
      continue;
    }
    if (end == -1) {
      matchedSlash = false;
      end = i + 1;
    }
    if (code == CHAR_DOT) {
      if (startDot == -1)
        startDot = i;
      else if (preDotState != 1)
        preDotState = 1;
    } else if (startDot != -1) {
      preDotState = -1;
    }
  }

  if (startDot == -1 ||
    end == -1 ||
    preDotState == 0 ||
    (preDotState == 1 &&
      startDot == end - 1 &&
      startDot == startPart + 1)) {
    if (end != -1) {
      if (startPart == 0 && isAbsolute)
        ret.base = ret.name = path.slice(1, end);
      else
        ret.base = ret.name = path.slice(startPart, end);
    }
  } else {
    if (startPart == 0 && isAbsolute) {
      ret.name = path.slice(1, startDot);
      ret.base = path.slice(1, end);
    } else {
      ret.name = path.slice(startPart, startDot);
      ret.base = path.slice(startPart, end);
    }
    ret.ext = path.slice(startDot, end);
  }

  if (startPart > 0)
    ret.dir = path.slice(0, startPart - 1);
  else if (isAbsolute)
    ret.dir = '/';

  return ret;
#endif
}
