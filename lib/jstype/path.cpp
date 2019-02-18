#include "path.h"

#ifdef _WIN32
String path::_sep = "\\";
#elif
String path::_sep = "/";
#endif
