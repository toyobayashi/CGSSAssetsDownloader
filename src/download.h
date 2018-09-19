#ifndef __DOWNLOAD_H__
#define __DOWNLOAD_H__

#include <iostream>

long get_file_size (const char*);

std::string Int_to_String (int);

void progress (double local, double current, double max, double speed);

void progress (double current, double max);

void download (std::string url, std::string path);

#endif // !__DOWNLOAD_H__
