#include "jsprocess.h"
#include "JSArray.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <cstdio>
#include <cstdlib>
#include <errno.h>

#ifdef _WIN32
#include <Windows.h>
#include <direct.h>
#include <io.h>
#include <wchar.h>
#include <process.h>
#else
#include <unistd.h>
#include <dirent.h>
#ifdef __APPLE__
#include <crt_externs.h>
#define environ (*_NSGetEnviron())
#else
extern char** environ;
#endif
#endif

int process::exitCode = 0;
Object<String> process::_env;

process::~process() {}

String process::cwd() {
#ifdef _WIN32
  wchar_t* buf;
  if ((buf = _wgetcwd(nullptr, 0)) == nullptr) {
    return "";
  }
  return String::fromWideChar(buf);
#else
  char* buf;
  if ((buf = getcwd(nullptr, 0) == nullptr)) {
    return "";
  }
  String res = buf;
  free(buf);
  return res;
#endif
}

bool process::chdir(const String& dirname) {
#ifdef _WIN32
  if (_wchdir(dirname.toCppWString().c_str()) != 0) {
    return false;
  }
  return true;
#else
  if (chdir(dirname.toWCppString().c_str()) != 0) {
    return false;
  }
  return true;
#endif
}

int process::pid() {
#ifdef _WIN32
  return _getpid();
#else
  return getpid();
#endif
}

void process::exit(const int code) {
  ::exit(code);
}

Object<String>& process::env() {
  if (Object<String>::keys(_env).length() == 0) {
#ifdef _WIN32
    wchar_t* environment = GetEnvironmentStringsW();
    if (environment == nullptr) return _env;
    wchar_t* p = environment;
    while (*p) {
      if (*p == L'=') {
        p += wcslen(p) + 1;
        continue;
      }
      String e = String::fromWideChar(p);
      Array<String> keyvalue = e.split("=");
      _env[keyvalue[0]] = keyvalue[1];

      p += wcslen(p) + 1;
    }
    FreeEnvironmentStringsW(environment);
#else
    int env_size = 0;
    while (environ[env_size]) {
      String e = environ[env_size];
      Array<String> keyvalue = e.split("=");
      _env[keyvalue[0]] = keyvalue[1];
      env_size++;
    }
#endif
  }
  return _env;
}

String process::version() {
  return "0.0.1-dev";
}
