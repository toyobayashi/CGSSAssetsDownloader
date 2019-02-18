#ifndef __FS_HPP__
#define __FS_HPP__

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
  fs() {};
  class _Stats {
  public:
    virtual ~_Stats() {};
#ifdef _WIN32
    _Stats(const struct _stat& info, int err = 0) {
      _dev = info.st_dev;
      _ino = info.st_ino;
      _mode = info.st_mode;
      _nlink = info.st_nlink;
      _gid = info.st_gid;
      _uid = info.st_uid;
      _rdev = info.st_rdev;
      _size = info.st_size;
      _atime = info.st_atime;
      _mtime = info.st_mtime;
      _ctime = info.st_ctime;
      _errno = err;
    }
#else
    Stats(const struct stat& info, int err = 0) {
      _dev = info.st_dev;
      _ino = info.st_ino;
      _mode = info.st_mode;
      _nlink = info.st_nlink;
      _gid = info.st_gid;
      _uid = info.st_uid;
      _rdev = info.st_rdev;
      _size = info.st_size;
      _atime = info.st_atime;
      _mtime = info.st_mtime;
      _ctime = info.st_ctime;
      _errno = err;
    }
#endif // _WIN32

    unsigned int dev() { return _dev; }
    unsigned short ino() { return _ino; }
    unsigned short mode() { return _mode; }
    short nlink() { return _nlink; }
    short gid() { return _gid; }
    short uid() { return _uid; }
    unsigned int rdev() { return _rdev; }
    long size() { return _size; }
    time_t atime() { return _atime; }
    time_t mtime() { return _mtime; }
    time_t ctime() { return _ctime; }

    bool isFile() { return _mode & S_IFREG; }
    bool isDirectory() { return _mode & S_IFDIR; }
    bool isSymbolicLink() {
#ifdef _WIN32
      return false;
#else
      return _mode & S_IFLNK;
#endif
    }
    bool isBlockDevice() {
#ifdef _WIN32
      return false;
#else
      return _mode & S_IFBLK;
#endif
    }
    bool isCharacterDevice() { return _mode & S_IFCHR; }
    bool isFIFO() {
#ifdef _WIN32
      return _mode & _S_IFIFO;
#else
      return _mode & S_IFIFO;
#endif
    }
    bool isSocket() {
#ifdef _WIN32
      return false;
#else
      return _mode & S_IFSOCK;
#endif
    }
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
  virtual ~fs() {};
  static Stats statSync(const String& filename) {

#ifdef _WIN32
    struct _stat info;
    String newPath = filename.replace(std::regex("/"), path::sep());
    int res = _wstat(newPath.toWCppString().c_str(), &info);
#else
    struct stat info;
    String newPath = path.replace(std::regex("\\"), path::sep());
    int res = stat(newPath.toCString(), &info);
#endif
    if (res != 0) {
      res = errno;
      info.st_size = 0;
      info.st_mode = 0;
      return Stats(info, res);
    }
    return info;
  }

  static bool existsSync(const String& filename) {
#ifdef _WIN32
    struct _stat info;
    String newPath = filename.replace(std::regex("/"), path::sep());
    int res = _wstat(newPath.toWCppString().c_str(), &info);
#else
    struct stat info;
    String newPath = path.replace(std::regex("\\"), path::sep());
    int res = stat(newPath.toCString(), &info);
#endif
    if (res != 0) {
      return ENOENT != errno;
    }
    return true;
  }

  static bool mkdirsSync(const String& path) {
    return fs::mkdirSync(path, 0777, true);
  }

  static bool mkdirSync(const String& path, int mode = 0777, bool recursive = false) {
    if (!recursive) {
      int res;
#ifdef _WIN32
      String newPath = path.replace(std::regex("/"), path::sep());
      if (fs::existsSync(newPath)) {
        return fs::statSync(newPath).isDirectory();
      }
      res = _wmkdir(newPath.toWCppString().c_str());
#else
      String newPath = path.replace(std::regex("\\"), path::sep());
      if (fs::existsSync(newPath)) {
        return fs::statSync(newPath).isDirectory();
      }
      res = mkdir(newPath.toCString(), mode);
#endif
      if (res != 0) {
        return false;
      } else {
        return true;
      }
    }

#ifdef _WIN32
    String newPath = path.replace(std::regex("/"), path::sep());
#else
    String newPath = path.replace(std::regex("\\"), path::sep());
#endif
    int sep = newPath.lastIndexOf(path::sep());
    String dirname;
    if (sep == -1) {
      dirname = ".";
    } else {
      dirname = newPath.substring(0, sep);
    }

    bool isDirectory = false;
    if (fs::existsSync(dirname)) {
      if (!fs::statSync(dirname).isDirectory()) {
        return false;
      } else {
        isDirectory = true;
      }
    } else {
      isDirectory = fs::mkdirSync(dirname, mode, recursive);
    }

    return isDirectory ? fs::mkdirSync(newPath) : isDirectory;
  }

  static bool unlinkSync(const String& path) {
#ifdef _WIN32
    String newPath = path.replace(std::regex("/"), path::sep());
    if (!fs::existsSync(newPath)) {
      return true;
    }

    int res = _wunlink(newPath.toWCppString().c_str());
    return res == 0;
#else
    String newPath = path.replace(std::regex("\\"), path::sep());
    if (!fs::existsSync(newPath)) {
      return true;
    }

    int res = unlink(newPath.toCString());
    return res == 0;
#endif
  }

  static FILE* openSync(const String& path, const String& mode) {
#ifdef _WIN32
    String newPath = path.replace(std::regex("/"), path::sep());
    return _wfopen(newPath.toWCppString().c_str(), mode.toWCppString().c_str());
#else
    String newPath = path.replace(std::regex("\\"), path::sep());
    return fopen(newPath.toCString(), mode.toCString());
#endif
  }

  static int closeSync(FILE* fp) {
    return fclose(fp);
  }

  static Array<String> readdirSync(const String& path) {
#ifdef _WIN32
    struct _wfinddata_t file;
    intptr_t hFile;
    String newPath = path.replace(std::regex("/"), path::sep());

    hFile = _wfindfirst((newPath + "\\*.*").toWCppString().c_str(), &file);
    if (hFile == -1) {
      return {};
    }

    Array<String> res;

    int len = WideCharToMultiByte(CP_UTF8, 0, file.name, -1, nullptr, 0, nullptr, nullptr);
    char* str = new char[len];
    WideCharToMultiByte(CP_UTF8, 0, file.name, -1, str, len, nullptr, nullptr);
    String item = str;
    delete[] str;
    str = nullptr;
    if (item != "." && item != "..") {
      res.push(item);
    }

    while (_wfindnext(hFile, &file) == 0) {
      int len = WideCharToMultiByte(CP_UTF8, 0, file.name, -1, nullptr, 0, nullptr, nullptr);
      char* str = new char[len];
      WideCharToMultiByte(CP_UTF8, 0, file.name, -1, str, len, nullptr, nullptr);
      String item = str;
      delete[] str;
      str = nullptr;
      if (item != "." && item != "..") {
        res.push(item);
      }
    }
    _findclose(hFile);

    return res;
#else
    String newPath = path.replace(std::regex("\\"), path::sep());
    DIR *dirp;
    struct dirent *direntp;
    int stats;
    char buf[80];
    if ((dirp = opendir(newPath.toCString())) == nullptr) {
      return {};
    }
    Array<String> res;
    while ((direntp = readdir(dirp)) != NULL) {
      String item = direntp->d_name;
      if (item != "." && item != "..") {
        res.push(item);
      }
    }
    closedir(dirp);
    return res;
#endif
  }
};

#endif // !__FS_H__
