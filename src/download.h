#ifndef __DOWNLOAD_H__
#define __DOWNLOAD_H__

#include <iostream>

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
  std::string path;
} progressInfo;

void progress (double local, double current, double max, double speed);

void progress (double current, double max);

bool download (std::string url, std::string path);

#endif // !__DOWNLOAD_H__
