//#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
//#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
//#endif

#include <sstream>
#include <math.h>
#include <time.h>
//#include <WinSock2.h>
#include "./download.h"
#include <curl.h>
// #pragma comment(lib, "ws2_32.lib")

using namespace std;

#define PROGRESS_LENGTH 38

long get_file_size (const char* strFileName) {
  struct _stat info;
  _stat(strFileName, &info);
  long size = info.st_size;
  return size;
}

//string Int_to_String (int n) {
//  ostringstream stream;
//  stream << n; 
//  return stream.str();
//}

void progress (double local, double current, double max, double speed) {
  double p_local = round(local / max * PROGRESS_LENGTH);
  double p_current = round(current / max * PROGRESS_LENGTH);
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
  for (int i = 0; i < (int)(PROGRESS_LENGTH - p_local - p_current); i++) {
    printf(" ");
  }
  printf("] ");
  printf("%5.2f%% ", percent);
  printf("%7.2lf KB/s", speed / 1024);
  printf("   \b\b\b");
}

void progress (double current, double max) {
  double p_current = floor(current / max * PROGRESS_LENGTH);
  double percent = current / max * 100;
  printf("Completed: %.0lf / %.0lf ", current, max);
  printf("[");
  for (int i = 0; i < (int)p_current - 1; i++) {
    printf("=");
  }
  printf(">");
  for (int i = 0; i < (int)(PROGRESS_LENGTH - p_current); i++) {
    printf(" ");
  }
  printf("] ");
  printf("%5.2f%% ", percent);
  printf("   \b\b\b");
}

const int BuffSize = 1024;

typedef struct {
  FILE* fp;
  long size;
  long sum;
  long total;
  int speed;
  double start_time;
  double last_time;
  double end_time;
  std::string headerString;
} progressInfo;

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

void download (string url, string path) {
  CURL* curl = curl_easy_init();
  struct curl_slist* headers = NULL;

  headers = curl_slist_append(headers, "X-Unity-Version: 2017.4.2f2");
  headers = curl_slist_append(headers, "Connection: Keep-Alive");
  headers = curl_slist_append(headers, "Accept-Encoding: gzip");
  headers = curl_slist_append(headers, "Accept: */*");
  headers = curl_slist_append(headers, "User-Agent: Dalvik/2.1.0 (Linux; U; Android 7.0; Nexus 42 Build/XYZZ1Y)");

  long size = get_file_size(path.c_str());

  if (size != 0) {
    headers = curl_slist_append(headers, (std::string("Range: bytes=") + std::to_string(size) + "-").c_str());
  }

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);
  //curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);

  progressInfo info;
  info.fp = fopen(path.c_str(), "ab+");
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

  CURLcode code = curl_easy_perform(curl);

  if (code != CURLE_OK) {
    printf("%s\n", curl_easy_strerror(code));
    fclose(info.fp);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return;
  }

  fclose(info.fp);
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  //size_t protocol = url.find("//") + 2;
  //string hostname = url.substr(protocol, url.substr(protocol).find_first_of('/'));

  //string route = url.substr(protocol).substr(url.substr(protocol).find_first_of('/'));

  //long size = get_file_size(path.c_str());

  //WSADATA WsaData;
  //if (WSAStartup(MAKEWORD(2, 2), &WsaData)) {
  //  printf("Init failed.\n");
  //  return;
  //}

  //SOCKET sockeId;
  //SOCKADDR_IN addr;
  //hostent *remoteHost;
  //remoteHost = gethostbyname(hostname.c_str());

  //if (-1 == (sockeId = socket(AF_INET, SOCK_STREAM, 0))) {
  //  printf("Create socket failed.\n");
  //  return;
  //}

  //addr.sin_addr.S_un.S_addr = *((unsigned long *)*remoteHost->h_addr_list);   //inet_addr("104.116.243.18")
  //addr.sin_family = AF_INET;
  //addr.sin_port = htons(80);

  //if (SOCKET_ERROR == connect(sockeId, (SOCKADDR *)&addr, sizeof(addr))) {
  //  printf("Connect server failed.\n");
  //  closesocket(sockeId);
  //  WSACleanup();
  //  return;
  //}

  //char* pReqHead = new char[BuffSize];
  //pReqHead[0] = '\0';
  //
  //strcat(pReqHead, "GET ");
  //strcat(pReqHead, route.c_str());
  //strcat(pReqHead, " HTTP/1.1\r\n");
 
  //string hosthead = "Host: " + hostname + "\r\n";

  //strcat(pReqHead, hosthead.c_str());
  //strcat(pReqHead, "Accept: */*\r\n");
  //strcat(pReqHead, "Connection: Keep-Alive\r\n");
  //strcat(pReqHead, "User-Agent: Dalvik/2.1.0 (Linux; U; Android 7.0; Nexus 42 Build/XYZZ1Y)\r\n");
  //strcat(pReqHead, "X-Unity-Version: 5.4.5p1\r\n");
  //strcat(pReqHead, "Accept-Encoding: gzip\r\n");
  //if (size != 0) {
  //  string range = "Range: bytes=" + Int_to_String(size) + "-\r\n";
  //  strcat(pReqHead, range.c_str());
  //}
  //strcat(pReqHead, "\r\n");

  //if (SOCKET_ERROR == send(sockeId, pReqHead, (int)strlen(pReqHead), 0)) {
  //  printf("Send data failed.\n");
  //  closesocket(sockeId);
  //  WSACleanup();
  //  delete pReqHead;
  //  return;
  //}

  //delete pReqHead;

  //FILE *fp;
  //fp = fopen(path.c_str(), "ab+");

  //if (NULL == fp) {
  //  printf("Create file failed.\n");
  //  closesocket(sockeId);
  //  WSACleanup();
  //  return;
  //}

  //char* buff = (char*)malloc(BuffSize * sizeof(char));
  //memset(buff, '\0', BuffSize);
  //int iRec = 1;
  //bool bStart = false;
  //int chars = 0;

  //string str;
  //while (!bStart) {
  //  iRec = recv(sockeId, buff, 1, 0);
  //  str += *buff;
  //  if (iRec < 0) {
  //    bStart = true;
  //  }
  //  switch (*buff)
  //  {
  //  case '\r':
  //    break;
  //  case '\n':
  //    if (chars == 0) {
  //      bStart = true;
  //    }
  //    chars = 0;
  //    break;
  //  default:
  //    chars++;
  //    break;
  //  }
  //}

  //int length = -1;
  //if (str.find("Content-Length: ") != str.npos) {
  //  string contentlength1 = str.substr(str.find("Content-Length: "));
  //  string contentlength2 = contentlength1.substr(0, contentlength1.find_first_of('\r'));
  //  string contentlength3 = contentlength2.substr(16);
  //  length = atoi(contentlength3.c_str());
  //}
  //else {
  //  printf("Download faild.\n");
  //  return;
  //}

  //if (size == length && str.find("Content-Range") == str.npos) {
  //  printf("File exists.\n");
  //}
  //else {
  //  int sum = 0;
  //  int speed = 0;
  //  double start_time = clock();
  //  double end_time = 0;
  //  int last_time = 0;
  //  do {
  //    iRec = recv(sockeId, buff, BuffSize, 0);
  //    if (iRec < 0) {
  //      break;
  //    }
  //    sum += iRec;
  //    speed += iRec;
  //    int now = clock();
  //    if (now - last_time > 500) {
  //      progress(size, sum, length + size, speed * 2);
  //      last_time = now;
  //      speed = 0;
  //    }

  //    fwrite(buff, iRec, 1, fp);
  //    if (sum == length) {
  //      end_time = clock();
  //      progress(size, sum, length + size, (double)sum / ((end_time - start_time) / 1000));
  //      closesocket(sockeId);
  //      WSACleanup();
  //      break;
  //    }
  //  } while (iRec > 0);
  //}

  //fclose(fp);
  //free(buff);
  //printf("\n\n");
}
