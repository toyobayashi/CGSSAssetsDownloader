#ifndef __CONSOLE_HPP__
#define __CONSOLE_HPP__

#include <iostream>
#include <cstdlib>
#include "JSString.h"
#include <map>

#ifdef _WIN32
#include <Windows.h>

#define COLOR_RED_BRIGHT (FOREGROUND_RED | FOREGROUND_INTENSITY)
#define COLOR_YELLOW_BRIGHT (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define COLOR_GREEN_BRIGHT (FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#else
#define COLOR_RED_BRIGHT ("\x1b[31;1m")
#define COLOR_YELLOW_BRIGHT ("\x1b[33;1m")
#define COLOR_GREEN_BRIGHT ("\x1b[32;1m")
#define COLOR_RESET ("\x1b[0m")
#endif

class console {
public:
  virtual ~console();

  static void write(char);
  static void write(const char*);
  static void write(const std::string&);
  static void write(const String&);

  template <typename T>
  static void write(const T& arg) {
    std::cout << arg;
  }

  template <typename... Args>
  static void write(const String& arg, Args... args) {
#ifdef _WIN32
    char* buf = _wideCharToMultiByteACP(arg.toWCppString());
    printf(buf, args...);
    delete[] buf;
#else
    printf(arg.toCString(), args...);
#endif
  }

  template <typename... Args>
  static void log(Args... args) {
    write(args...);
    printf("\n");
  }

  template <typename... Args>
  static void info(Args... args) {
#ifdef _WIN32
    WORD originalAttr = _setConsoleTextAttribute(_consoleErrorHandle, COLOR_GREEN_BRIGHT);
    write(args...);
    printf("\n");
    _setConsoleTextAttribute(_consoleErrorHandle, originalAttr);
#else
    printf(COLOR_GREEN_BRIGHT);
    write(args...);
    printf("\n");
    printf(COLOR_RESET);
#endif
  }

  static void warn(char);
  static void warn(const char*);
  static void warn(const std::string&);
  static void warn(const String&);

  template <typename T>
  static void warn(const T& arg) {
#ifdef _WIN32
    WORD originalAttr = _setConsoleTextAttribute(_consoleErrorHandle, COLOR_YELLOW_BRIGHT);
    std::cerr << arg << std::endl;
    _setConsoleTextAttribute(_consoleErrorHandle, originalAttr);
#else
    std::cerr << COLOR_YELLOW_BRIGHT << arg << COLOR_RESET << std::endl;
#endif
  }

  template <typename... Args>
  static void warn(const String& arg, Args... args) {
#ifdef _WIN32
    char* buf = _wideCharToMultiByteACP(arg.toWCppString());
    char* res = new char[strlen(buf) + 2]{ 0 };
    sprintf(res, buf, args...);
    WORD originalAttr = _setConsoleTextAttribute(_consoleErrorHandle, COLOR_YELLOW_BRIGHT);
    std::cerr << res << std::endl;
    _setConsoleTextAttribute(_consoleErrorHandle, originalAttr);
    delete[] res;
    delete[] buf;
#else
    char* res = new char[arg.byteLength() + 2]{ 0 };
    sprintf(res, arg.toCString(), args...);
    std::cerr << COLOR_YELLOW_BRIGHT << res << COLOR_RESET << std::endl;
    delete[] res;
#endif
  }

  static void error(char);
  static void error(const char*);
  static void error(const std::string&);
  static void error(const String&);

  template <typename T>
  static void error(const T& arg) {
#ifdef _WIN32
    WORD originalAttr = _setConsoleTextAttribute(_consoleErrorHandle, COLOR_RED_BRIGHT);
    std::cerr << arg << std::endl;
    _setConsoleTextAttribute(_consoleErrorHandle, originalAttr);
#else
    std::cerr << COLOR_RED_BRIGHT << arg << COLOR_RESET << std::endl;
#endif
  }

  template <typename... Args>
  static void error(const String& arg, Args... args) {
#ifdef _WIN32
    char* buf = _wideCharToMultiByteACP(arg.toWCppString());
    char* res = new char[strlen(buf) + 2]{ 0 };
    sprintf(res, buf, args...);
    WORD originalAttr = _setConsoleTextAttribute(_consoleErrorHandle, COLOR_RED_BRIGHT);
    std::cerr << res << std::endl;
    _setConsoleTextAttribute(_consoleErrorHandle, originalAttr);
    delete[] res;
    delete[] buf;
#else
    char* res = new char[arg.byteLength() + 2]{ 0 };
    sprintf(res, arg.toCString(), args...);
    std::cerr << COLOR_RED_BRIGHT << res << COLOR_RESET << std::endl;
    delete[] res;
#endif
  }

  static void time(const String& label = "default");
  static void timeEnd(const String& label = "default");

  static void clear();
  static void clearLine(uint16_t lineNumber);
private:
  console();

#ifdef _WIN32
  static HANDLE _consoleHandle;
  static HANDLE _consoleErrorHandle;
  static char* _wideCharToMultiByteACP(const std::wstring&);
  static WORD _setConsoleTextAttribute(HANDLE, WORD);
#endif

  static std::map<std::string, double> _timemap;
  static double _getTime();
};

#endif