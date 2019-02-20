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

  static bool rmdirSync(const String& path) {
#ifdef _WIN32
    String newPath = path.replace(std::regex("/"), path::sep());
    if (!fs::existsSync(newPath)) {
      return true;
    }

    int res = _wrmdir(newPath.toWCppString().c_str());
    return res == 0;
#else
    String newPath = path.replace(std::regex("\\"), path::sep());
    if (!fs::existsSync(newPath)) {
      return true;
    }

    int res = rmdir(newPath.toCString());
    return res == 0;
#endif
  }

  static bool renameSync(const String& source, const String& dest) {
#ifdef _WIN32
    String newSource = source.replace(std::regex("/"), path::sep());
    String newDest = dest.replace(std::regex("/"), path::sep());

    if (!fs::existsSync(newSource)) {
      return false;
    }

    if (fs::existsSync(newDest)) {
      if (!fs::renameSync(newDest, newDest + ".tmp")) {
        return false;
      }
    }

    int res = _wrename(newSource.toWCppString().c_str(), newDest.toWCppString().c_str());
    if (res == 0) {
      fs::removeSync(newDest + ".tmp");
      return true;
    } else {
      fs::renameSync(newDest + ".tmp", newDest);
      return false;
    }
#else
    String newSource = source.replace(std::regex("\\"), path::sep());
    String newDest = dest.replace(std::regex("\\"), path::sep());

    if (!fs::existsSync(newSource)) {
      return false;
    }
    if (fs::existsSync(newDest)) {
      if (!fs::unlinkSync(newDest)) {
        return false;
      }
    }

    int res = rename(newSource.toCString(), newDest.toCString());
    return res == 0;
#endif
  }

  static bool copySync(const String& source, const String& dest) {
#ifdef _WIN32
    String newSource = source.replace(std::regex("/"), path::sep());
    String newDest = dest.replace(std::regex("/"), path::sep());
#else
    String newSource = source.replace(std::regex("\\"), path::sep());
    String newDest = dest.replace(std::regex("\\"), path::sep());
#endif

    if (!fs::existsSync(newSource)) {
      return false;
    }

    fs::Stats sourceStat = fs::statSync(newSource);

    if (fs::existsSync(newDest)) {
      if (!fs::removeSync(newDest)) {
        return false;
      }
    }

    if (sourceStat.isDirectory()) {
      Array<String> items = fs::readdirSync(newSource);
      
      if (!fs::mkdirsSync(newDest)) {
        return false;
      }
      bool res = true;
      for (int i = 0; i < items.length(); i++) {
        String _source = path::join(newSource, items[i]);
        String _dest = path::join(newDest, items[i]);
        if (!fs::copySync(_source, _dest)) res = false;
      }

      return res;
    } else {
      FILE* s = fs::openSync(newSource, "rb+");
      FILE* d = fs::openSync(newDest, "wb+");
      byte buf[8192];
      int read;
      while ((read = (int)fread(buf, sizeof(byte), 8192, s)) > 0) {
        fwrite(buf, sizeof(byte), read, d);
      }
      fs::closeSync(s);
      fs::closeSync(d);
      return true;
    }

    return false;

  }

  static bool removeSync(const String& path) {
#ifdef _WIN32
    String newPath = path.replace(std::regex("/"), path::sep());
#else
    String newPath = path.replace(std::regex("\\"), path::sep());
#endif
    if (!fs::existsSync(newPath)) {
      return true;
    }

    fs::Stats stat = fs::statSync(newPath);
    if (stat.isDirectory()) {
      Array<String> items = fs::readdirSync(newPath);
      if (items.length() != 0) {
        bool res = true;
        for (int i = 0; i < items.length(); i++) {
          const String& item = items[i];
          if (!fs::removeSync(path::join(newPath, item))) {
            res = false;
          }
        }
        bool lastResult = rmdirSync(newPath);
        return res ? lastResult : res;
      } else {
        return rmdirSync(newPath);
      }
    }
      
    return unlinkSync(newPath);
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
