#include <sstream>
#include <math.h>
#include <time.h>
#include "./download.h"
#include <curl.h>
#include "../lib/jstype/fs.h"
#include "../lib/jstype/console.h"

using namespace std;

// #define PROGRESS_LENGTH 38
#define PROGRESS 50

#ifdef _WIN32
static HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
#endif

static int getTerminalWidth() {
#ifdef _WIN32
  CONSOLE_SCREEN_BUFFER_INFO bInfo;
  GetConsoleScreenBufferInfo(hOutput, &bInfo);
  return bInfo.dwSize.X;
#else
  return PROGRESS + 38;
#endif
}

static int getTerminalCursorPositionToRight() {
#ifdef _WIN32
  CONSOLE_SCREEN_BUFFER_INFO bInfo;
  GetConsoleScreenBufferInfo(hOutput, &bInfo);
  // CloseHandle(hOutput);
  return bInfo.dwSize.X - bInfo.dwCursorPosition.X;
#else
  return 5;
#endif
}

void progress (double local, double current, double max, double speed) {
  int progressLength = getTerminalWidth() - PROGRESS;
  double p_local = round(local / max * progressLength);
  double p_current = round(current / max * progressLength);
  double percent = (local + current) / max * 100;
  printf("\r");
  printf("%.2lf / %.2lf MB ", (local + current) / 1024 / 1024, max / 1024 / 1024);
  printf("[");
  for (int i = 0; i < (int)p_local; i++) {
    printf("+");
  }
  for (int i = 0; i < (int)p_current/* - 1*/; i++) {
    printf("=");
  }
  printf(">");
  for (int i = 0; i < (int)(progressLength - p_local - p_current); i++) {
    printf(" ");
  }
  printf("] ");
  printf("%5.2f%% ", percent);
  printf("%7.2lf KB/s", speed / 1024);
  int marginRight = getTerminalCursorPositionToRight();
  printf((String(" ").repeat(marginRight) + String("\b").repeat(marginRight)).toCString());
}

void progress (double current, double max) {
  int progressLength = getTerminalWidth() - PROGRESS;
  double p_current = floor(current / max * progressLength);
  double percent = current / max * 100;
  printf("Completed: %.0lf / %.0lf ", current, max);
  printf("[");
  for (int i = 0; i < (int)p_current - 1; i++) {
    printf("=");
  }
  printf(">");
  for (int i = 0; i < (int)(progressLength - p_current); i++) {
    printf(" ");
  }
  printf("] ");
  printf("%5.2f%% ", percent);
  int marginRight = getTerminalCursorPositionToRight();
  printf((String(" ").repeat(marginRight) + String("\b").repeat(marginRight)).toCString());
}

const int BuffSize = 1024;

static size_t onDataString(void* buffer, size_t size, size_t nmemb, progressInfo* userp) {
  char* d = (char*)buffer;
  int result = 0;
  userp->headerString.append(d, size * nmemb);
  result = size * nmemb;
  return result;
}

static size_t onDataWrite(void* buffer, size_t size, size_t nmemb, progressInfo* userp) {
  if (userp->total == -1) {
    std::cout << userp->headerString << std::endl;
    string contentlength1 = userp->headerString.substr(userp->headerString.find("Content-Length: "));
    string contentlength2 = contentlength1.substr(0, contentlength1.find_first_of('\r'));
    string contentlength3 = contentlength2.substr(16);
    int length = atoi(contentlength3.c_str());
    userp->total = length + userp->size;
  }

  if (userp->fp == nullptr) {
    userp->fp = fs::openSync(userp->path.c_str(), "ab+");
    if (!(userp->fp)) {
      return size * nmemb;
    }
  }

  size_t iRec = fwrite(buffer, size, nmemb, userp->fp);
  if (iRec < 0) {
    return iRec;
  }
  userp->sum += iRec;
  userp->speed += iRec;
  int now = clock();
  if (now - userp->last_time > 500) {
    progress(userp->size, userp->sum, userp->total, userp->speed * 2);
    userp->last_time = now;
    userp->speed = 0;
  }

  if (userp->sum == userp->total - userp->size) {
    userp->end_time = clock();
    progress(userp->size, userp->sum, userp->total, (double)userp->sum / ((userp->end_time - userp->start_time) / 1000));
  }
  return iRec;
}

bool download (string url, string path) {
  CURL* curl = curl_easy_init();
  struct curl_slist* headers = NULL;

  headers = curl_slist_append(headers, "X-Unity-Version: 2017.4.2f2");
  headers = curl_slist_append(headers, "Connection: Keep-Alive");
  headers = curl_slist_append(headers, "Accept-Encoding: gzip");
  headers = curl_slist_append(headers, "Accept: */*");
  headers = curl_slist_append(headers, "User-Agent: Dalvik/2.1.0 (Linux; U; Android 7.0; Nexus 42 Build/XYZZ1Y)");

  // long size = get_file_size(path.c_str());
  auto a = fs::statSync(path);
  long size = a.size();
  // std::cout << size << std::endl;

  if (size != 0) {
    headers = curl_slist_append(headers, (std::string("Range: bytes=") + std::to_string(size) + "-").c_str());
  }

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);
  //curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);

  progressInfo info;
  info.path = path;
  info.fp = nullptr;
  info.size = size;
  info.sum = 0;
  info.speed = 0;
  info.start_time = clock();
  info.end_time = 0;
  info.last_time = 0;
  info.total = -1;
  info.headerString = "";

  curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, &onDataString);
  curl_easy_setopt(curl, CURLOPT_HEADERDATA, &info);

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &onDataWrite);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &info);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

  console::log(String("GET ") + url + " HTTP/1.1\n");
  struct curl_slist* ph = headers;
  while (ph != NULL) {
    console::log(ph->data);
    ph = ph->next;
  }
  console::log("");

  CURLcode code = curl_easy_perform(curl);

  if (code != CURLE_OK) {
    printf("%s\n", curl_easy_strerror(code));
    if (info.fp != nullptr) {
      fs::closeSync(info.fp);
      info.fp = nullptr;
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return false;
  }

  if (info.fp != nullptr) {
    fs::closeSync(info.fp);
    info.fp = nullptr;
  }
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  return true;
}
