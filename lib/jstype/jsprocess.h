#ifndef __JS_PROCESS_H__
#define __JS_PROCESS_H__

#include "JSString.h"
#include "JSObject.hpp"
#include <map>

class process {
private:
  process();
  static Object<String> _env;

public:
  virtual ~process();

  static String cwd();
  static bool chdir(const String&);
  static String version();
  static int pid();
  static void exit(int const code = process::exitCode);
  static Object<String>& env();

  static int exitCode;
};

#endif // !__PROCESS_H__
