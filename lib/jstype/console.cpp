#include "console.h"
#include "Date.h"
#include <cmath>

std::map<std::string, long long> console::_timemap;

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

console::~console() {}

void console::write(const char* arg) {
#ifdef _WIN32
  char* buf = _wideCharToMultiByteACP(String(arg).toCppWString());
  printf(buf);
  delete[] buf;
#else
  printf(arg);
#endif
}

void console::write(bool arg) {
  printf(arg ? "true" : "false");
}

void console::write(char arg) {
#ifdef _WIN32
  char* buf = _wideCharToMultiByteACP(String(arg).toCppWString());
  printf(buf);
  delete[] buf;
#else
  printf(String(arg).toCString());
#endif
}

void console::write(const std::string& arg) {
#ifdef _WIN32
  char* buf = _wideCharToMultiByteACP(String(arg).toCppWString());
  printf(buf);
  delete[] buf;
#else
  printf(arg.c_str());
#endif
}

void console::write(const String& arg) {
#ifdef _WIN32
  char* buf = _wideCharToMultiByteACP(arg.toCppWString());
  printf(buf);
  delete[] buf;
#else
  printf(arg.toCString());
#endif
}

void console::write(const Array<String>& arr) {
  printf("[ ");
  int len = arr.length();
  if (len >= 1) {
    write(String("\"") + arr[0] + "\"");
    for (int i = 1; i < len; i++) {
      write(String(", ") + "\"" + arr[i] + "\"");
    }
  }
  printf(" ]");
}

void console::write(const Buffer& buf) {
  const String& hex = buf.toString("hex");
  int len = hex.byteLength();
  printf("<Buffer ");
  if (len >= 2) {
    String res = hex.substring(0, 2);
    for (int i = 2; i < len; i += 2) {
      res += " ";
      res += hex.substring(i, i + 2);
    }
    printf(res.toCString());
  }
  printf(">");
}

void console::warn(const char* arg) {
#ifdef _WIN32
  char* buf = _wideCharToMultiByteACP(String(arg).toCppWString());
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
  char* buf = _wideCharToMultiByteACP(String(arg).toCppWString());
  WORD originalAttr = _setConsoleTextAttribute(_consoleErrorHandle, COLOR_YELLOW_BRIGHT);
  std::cerr << buf << std::endl;
  _setConsoleTextAttribute(_consoleErrorHandle, originalAttr);
  delete[] buf;
#else
  std::cerr << COLOR_YELLOW_BRIGHT << arg << COLOR_RESET << std::endl;
#endif
}

void console::warn(bool arg) {
#ifdef _WIN32
  WORD originalAttr = _setConsoleTextAttribute(_consoleErrorHandle, COLOR_YELLOW_BRIGHT);
  std::cerr << (arg ? "true" : "false") << std::endl;
  _setConsoleTextAttribute(_consoleErrorHandle, originalAttr);
#else
  std::cerr << COLOR_YELLOW_BRIGHT << (arg ? "true" : "false") << COLOR_RESET << std::endl;
#endif
}

void console::warn(const std::string& arg) {
#ifdef _WIN32
  char* buf = _wideCharToMultiByteACP(String(arg).toCppWString());
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
  char* buf = _wideCharToMultiByteACP(arg.toCppWString());
  WORD originalAttr = _setConsoleTextAttribute(_consoleErrorHandle, COLOR_YELLOW_BRIGHT);
  std::cerr << buf << std::endl;
  _setConsoleTextAttribute(_consoleErrorHandle, originalAttr);
  delete[] buf;
#else
  std::cerr << COLOR_YELLOW_BRIGHT << arg.toCppString() << COLOR_RESET << std::endl;
#endif
}

void console::warn(const Array<String>& arr) {
#ifdef _WIN32
  WORD originalAttr = _setConsoleTextAttribute(_consoleErrorHandle, COLOR_YELLOW_BRIGHT);
#else
  std::cerr << COLOR_YELLOW_BRIGHT;
#endif
  std::cerr << "[ ";
  int len = arr.length();
  if (len >= 1) {
    String res = String("\"") + arr[0] + "\"";
    for (int i = 1; i < len; i++) {
      res += ", ";
      res += (String("\"") + arr[i] + "\"");
    }
    char* buf = _wideCharToMultiByteACP(res.toCppWString());
    std::cerr << buf;
    delete[] buf;
  }
  std::cerr << " ]" << std::endl;
#ifdef _WIN32
  _setConsoleTextAttribute(_consoleErrorHandle, originalAttr);
#else
  std::cerr << COLOR_RESET;
#endif
}

void console::warn(const Buffer& buf) {
  const String& hex = buf.toString("hex");
  int len = hex.byteLength();
#ifdef _WIN32
  WORD originalAttr = _setConsoleTextAttribute(_consoleErrorHandle, COLOR_YELLOW_BRIGHT);
#else
  std::cerr << COLOR_YELLOW_BRIGHT;
#endif
  std::cerr << "<Buffer ";
  if (len >= 2) {
    String res = hex.substring(0, 2);
    for (int i = 2; i < len; i += 2) {
      res += " ";
      res += hex.substring(i, i + 2);
    }
    std::cerr << res.toCppString();
  }
  std::cerr << ">" << std::endl;
#ifdef _WIN32
  _setConsoleTextAttribute(_consoleErrorHandle, originalAttr);
#else
  std::cerr << COLOR_RESET;
#endif
}

void console::error(const char* arg) {
#ifdef _WIN32
  char* buf = _wideCharToMultiByteACP(String(arg).toCppWString());
  WORD originalAttr = _setConsoleTextAttribute(_consoleErrorHandle, COLOR_RED_BRIGHT);
  std::cerr << buf << std::endl;
  _setConsoleTextAttribute(_consoleErrorHandle, originalAttr);
  delete[] buf;
#else
  std::cerr << COLOR_RED_BRIGHT << arg << COLOR_RESET << std::endl;
#endif
}

void console::error(bool arg) {
#ifdef _WIN32
  WORD originalAttr = _setConsoleTextAttribute(_consoleErrorHandle, COLOR_RED_BRIGHT);
  std::cerr << (arg ? "true" : "false") << std::endl;
  _setConsoleTextAttribute(_consoleErrorHandle, originalAttr);
#else
  std::cerr << COLOR_RED_BRIGHT << (arg ? "true" : "false") << COLOR_RESET << std::endl;
#endif
}

void console::error(char arg) {
#ifdef _WIN32
  char* buf = _wideCharToMultiByteACP(String(arg).toCppWString());
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
  char* buf = _wideCharToMultiByteACP(String(arg).toCppWString());
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
  char* buf = _wideCharToMultiByteACP(arg.toCppWString());
  WORD originalAttr = _setConsoleTextAttribute(_consoleErrorHandle, COLOR_RED_BRIGHT);
  std::cerr << buf << std::endl;
  _setConsoleTextAttribute(_consoleErrorHandle, originalAttr);
  delete[] buf;
#else
  std::cerr << COLOR_RED_BRIGHT << arg.toCppString() << COLOR_RESET << std::endl;
#endif
}

void console::error(const Array<String>& arr) {
#ifdef _WIN32
  WORD originalAttr = _setConsoleTextAttribute(_consoleErrorHandle, COLOR_RED_BRIGHT);
#else
  std::cerr << COLOR_RED_BRIGHT;
#endif
  std::cerr << "[ ";
  int len = arr.length();
  if (len >= 1) {
    String res = String("\"") + arr[0] + "\"";
    for (int i = 1; i < len; i++) {
      res += ", ";
      res += (String("\"") + arr[i] + "\"");
    }
    char* buf = _wideCharToMultiByteACP(res.toCppWString());
    std::cerr << buf;
    delete[] buf;
  }
  std::cerr << " ]" << std::endl;
#ifdef _WIN32
  _setConsoleTextAttribute(_consoleErrorHandle, originalAttr);
#else
  std::cerr << COLOR_RESET;
#endif
}

void console::error(const Buffer& buf) {
  const String& hex = buf.toString("hex");
  int len = hex.byteLength();
#ifdef _WIN32
  WORD originalAttr = _setConsoleTextAttribute(_consoleErrorHandle, COLOR_RED_BRIGHT);
#else
  std::cerr << COLOR_RED_BRIGHT;
#endif
  std::cerr << "<Buffer ";
  if (len >= 2) {
    String res = hex.substring(0, 2);
    for (int i = 2; i < len; i += 2) {
      res += " ";
      res += hex.substring(i, i + 2);
    }
    std::cerr << res.toCppString();
  }
  std::cerr << ">" << std::endl;
#ifdef _WIN32
  _setConsoleTextAttribute(_consoleErrorHandle, originalAttr);
#else
  std::cerr << COLOR_RESET;
#endif
}

void console::time(const String& label) {
  _timemap[label.toCppString()] = Date::nowEx();
}

void console::timeEnd(const String& label) {
  std::map<std::string, long long>::iterator it = _timemap.find(label.toCppString());
  if (it != _timemap.end()) {
    long long end = Date::nowEx();
    long long res = end - _timemap[label.toCppString()];
    double result = (double)res / 1000;
    char buf[16] = {0};
    sprintf(buf, "%.3lf", result);
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
