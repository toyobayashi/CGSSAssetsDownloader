#include "console.h"
#include <cmath>

#ifndef _WIN32
#include <sys/times.h>
#else
#define EPOCHFILETIME (116444736000000000UL)
#endif

std::map<std::string, double> console::_timemap;

#ifdef _WIN32
HANDLE console::_consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
HANDLE console::_consoleErrorHandle = GetStdHandle(STD_ERROR_HANDLE);

char* console::_wideCharToMultiByteACP(const std::wstring& wstr) {
  int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
  char* buf = new char[len]{0};
  WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, buf, len, nullptr, nullptr);
  return buf;
}

WORD console::_setConsoleTextAttribute(HANDLE hConsole, WORD wAttr) {
  CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
  if (!GetConsoleScreenBufferInfo(hConsole, &csbiInfo)) return FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
  WORD originalAttr = csbiInfo.wAttributes;
  SetConsoleTextAttribute(hConsole, wAttr);
  return originalAttr;
}
#endif

void console::clear() {
#ifdef _WIN32
  COORD coordScreen = { 0, 0 };
  DWORD cCharsWritten;
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  DWORD dwConSize;

  if (!GetConsoleScreenBufferInfo(_consoleHandle, &csbi)) {
    return;
  }

  dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

  if (!FillConsoleOutputCharacter(_consoleHandle,
    (TCHAR) ' ',
    dwConSize,
    coordScreen,
    &cCharsWritten))
  {
    return;
  }

  if (!GetConsoleScreenBufferInfo(_consoleHandle, &csbi)) {
    return;
  }

  if (!FillConsoleOutputAttribute(_consoleHandle,
    csbi.wAttributes,
    dwConSize,
    coordScreen,
    &cCharsWritten))
  {
    return;
  }

  SetConsoleCursorPosition(_consoleHandle, coordScreen);
#else
  system("clear");
#endif
}

void console::write(const char* arg) {
#ifdef _WIN32
  char* buf = _wideCharToMultiByteACP(String(arg).toWCppString());
  printf(buf);
  delete[] buf;
#else
  printf(arg);
#endif
}

void console::write(char arg) {
#ifdef _WIN32
  char* buf = _wideCharToMultiByteACP(String(arg).toWCppString());
  printf(buf);
  delete[] buf;
#else
  printf(String(arg).toCString());
#endif
}

void console::write(const std::string& arg) {
#ifdef _WIN32
  char* buf = _wideCharToMultiByteACP(String(arg).toWCppString());
  printf(buf);
  delete[] buf;
#else
  printf(arg.c_str());
#endif
}

void console::write(const String& arg) {
#ifdef _WIN32
  char* buf = _wideCharToMultiByteACP(arg.toWCppString());
  printf(buf);
  delete[] buf;
#else
  printf(arg.toCString());
#endif
}

void console::warn(const char* arg) {
#ifdef _WIN32
  char* buf = _wideCharToMultiByteACP(String(arg).toWCppString());
  WORD originalAttr = _setConsoleTextAttribute(_consoleErrorHandle, COLOR_YELLOW_BRIGHT);
  std::cerr << buf << std::endl;
  _setConsoleTextAttribute(_consoleErrorHandle, originalAttr);
  delete[] buf;
#else
  std::cerr << COLOR_YELLOW_BRIGHT << arg << COLOR_RESET << std::endl;
#endif
}

void console::warn(char arg) {
#ifdef _WIN32
  char* buf = _wideCharToMultiByteACP(String(arg).toWCppString());
  WORD originalAttr = _setConsoleTextAttribute(_consoleErrorHandle, COLOR_YELLOW_BRIGHT);
  std::cerr << buf << std::endl;
  _setConsoleTextAttribute(_consoleErrorHandle, originalAttr);
  delete[] buf;
#else
  std::cerr << COLOR_YELLOW_BRIGHT << arg << COLOR_RESET << std::endl;
#endif
}

void console::warn(const std::string& arg) {
#ifdef _WIN32
  char* buf = _wideCharToMultiByteACP(String(arg).toWCppString());
  WORD originalAttr = _setConsoleTextAttribute(_consoleErrorHandle, COLOR_YELLOW_BRIGHT);
  std::cerr << buf << std::endl;
  _setConsoleTextAttribute(_consoleErrorHandle, originalAttr);
  delete[] buf;
#else
  std::cerr << COLOR_YELLOW_BRIGHT << arg << COLOR_RESET << std::endl;
#endif
}

void console::warn(const String& arg) {
#ifdef _WIN32
  char* buf = _wideCharToMultiByteACP(arg.toWCppString());
  WORD originalAttr = _setConsoleTextAttribute(_consoleErrorHandle, COLOR_YELLOW_BRIGHT);
  std::cerr << buf << std::endl;
  _setConsoleTextAttribute(_consoleErrorHandle, originalAttr);
  delete[] buf;
#else
  std::cerr << COLOR_YELLOW_BRIGHT << arg.toCppString() << COLOR_RESET << std::endl;
#endif
}

void console::error(const char* arg) {
#ifdef _WIN32
  char* buf = _wideCharToMultiByteACP(String(arg).toWCppString());
  WORD originalAttr = _setConsoleTextAttribute(_consoleErrorHandle, COLOR_RED_BRIGHT);
  std::cerr << buf << std::endl;
  _setConsoleTextAttribute(_consoleErrorHandle, originalAttr);
  delete[] buf;
#else
  std::cerr << COLOR_RED_BRIGHT << arg << COLOR_RESET << std::endl;
#endif
}

void console::error(char arg) {
#ifdef _WIN32
  char* buf = _wideCharToMultiByteACP(String(arg).toWCppString());
  WORD originalAttr = _setConsoleTextAttribute(_consoleErrorHandle, COLOR_RED_BRIGHT);
  std::cerr << buf << std::endl;
  _setConsoleTextAttribute(_consoleErrorHandle, originalAttr);
  delete[] buf;
#else
  std::cerr << COLOR_RED_BRIGHT << arg << COLOR_RESET << std::endl;
#endif
}

void console::error(const std::string& arg) {
#ifdef _WIN32
  char* buf = _wideCharToMultiByteACP(String(arg).toWCppString());
  WORD originalAttr = _setConsoleTextAttribute(_consoleErrorHandle, COLOR_RED_BRIGHT);
  std::cerr << buf << std::endl;
  _setConsoleTextAttribute(_consoleErrorHandle, originalAttr);
  delete[] buf;
#else
  std::cerr << COLOR_RED_BRIGHT << arg << COLOR_RESET << std::endl;
#endif
}

void console::error(const String& arg) {
#ifdef _WIN32
  char* buf = _wideCharToMultiByteACP(arg.toWCppString());
  WORD originalAttr = _setConsoleTextAttribute(_consoleErrorHandle, COLOR_RED_BRIGHT);
  std::cerr << buf << std::endl;
  _setConsoleTextAttribute(_consoleErrorHandle, originalAttr);
  delete[] buf;
#else
  std::cerr << COLOR_RED_BRIGHT << arg.toCppString() << COLOR_RESET << std::endl;
#endif
}

double console::_getTime() {
#ifdef _WIN32
  FILETIME ft;
  LARGE_INTEGER li;
  double tt = 0;
  GetSystemTimeAsFileTime(&ft);
  li.LowPart = ft.dwLowDateTime;
  li.HighPart = ft.dwHighDateTime;
  tt = (double)((li.QuadPart - EPOCHFILETIME) / 10) / 1000;
  return tt;
#else
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 + (double)tv.tv_usec / 1000;
#endif
}

void console::time(const String& label) {
  _timemap[label.toCppString()] = _getTime();
}

void console::timeEnd(const String& label) {
  std::map<std::string, double>::iterator it = _timemap.find(label.toCppString());
  if (it != _timemap.end()) {
    double end = _getTime();
    double res = end - _timemap[label.toCppString()];
    res = round(res * 1000) / 1000;
    char buf[16] = {0};
    sprintf(buf, "%.3lf", res);
    log(label + ": " + buf + "ms");
    _timemap.erase(it);
  } else {
    log(String("Warning: No such label \"") + label + "\" for console::timeEnd()");
  }
}

void console::clearLine(uint16_t lineNumber) {
#ifdef _WIN32
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  if (!GetConsoleScreenBufferInfo(_consoleHandle, &csbi)) return;
  short tmp = csbi.dwCursorPosition.Y - lineNumber;
  COORD targetFirstCellPosition = { 0, tmp < 0 ? 0 : tmp };
  DWORD size = csbi.dwSize.X * lineNumber;
  DWORD cCharsWritten;

  if (!FillConsoleOutputCharacter(_consoleHandle, (TCHAR)' ', size, targetFirstCellPosition, &cCharsWritten)) return;
  if (!GetConsoleScreenBufferInfo(_consoleHandle, &csbi)) return;
  if (!FillConsoleOutputAttribute(_consoleHandle, csbi.wAttributes, size, targetFirstCellPosition, &cCharsWritten)) return;
  SetConsoleCursorPosition(_consoleHandle, targetFirstCellPosition);
#else
  for (int i = 0; i < lineNumber; i++) {
    printf("\x1b[666D\x1b[0K\x1b[1A");
  }
  printf("\x1b[666D\x1b[0K");
#endif
}
