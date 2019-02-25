#ifndef __FS_H__
#define __FS_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <cstdio>
#include <cstdlib>
#include <errno.h>
#include "JSString.h"
#include "JSArray.hpp"
#include "path.h"

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <wchar.h>
#else
#include <unistd.h>
#include <dirent.h>
#endif

class fs {
private:
  fs();
  class _Stats {
  public:
    virtual ~_Stats();
#ifdef _WIN32
    _Stats(const struct _stat& info, int err = 0);
#else
    _Stats(const struct stat& info, int err = 0);
#endif

    unsigned int dev();
    unsigned short ino();
    unsigned short mode();
    short nlink();
    short gid();
    short uid();
    unsigned int rdev();
    long size();
    time_t atime();
    time_t mtime();
    time_t ctime();

    bool isFile();
    bool isDirectory();
    bool isSymbolicLink();
    bool isBlockDevice();
    bool isCharacterDevice();
    bool isFIFO();
    bool isSocket();
  private:
    unsigned int _dev;
    unsigned short _ino;
    unsigned short _mode;
    short _nlink;
    short _gid;
    short _uid;
    unsigned int _rdev;
    long _size;
    time_t _atime;
    time_t _mtime;
    time_t _ctime;
    int _errno;
  };

public:
  typedef _Stats Stats;
  virtual ~fs();
  static Stats statSync(const String&);
  static bool existsSync(const String&);
  static bool mkdirsSync(const String&);
  static bool mkdirSync(const String&, int mode = 0777, bool recursive = false);
  static bool unlinkSync(const String&);
  static bool rmdirSync(const String&);
  static bool renameSync(const String&, const String&);
  static bool copySync(const String&, const String&);
  static bool removeSync(const String&);
  static Array<String> readdirSync(const String& path);
  static FILE* openSync(const String&, const String&);
  static int closeSync(FILE*);
};

#endif // !__FS_H__
