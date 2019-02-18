#ifndef __PATH_HPP__
#define __PATH_HPP__

#include "JSString.h"

class path {
public:
  virtual ~path() {};
  static String sep() { return _sep; }

private:
  path() {};
  static String _sep;
};

#endif
