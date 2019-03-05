#include "Date.h"

#ifndef _WIN32
#include <sys/times.h>
#else
#include <Windows.h>
#define EPOCHFILETIME (116444736000000000UL)
#endif

long long Date::now() {
#ifdef _WIN32
  FILETIME ft;
  LARGE_INTEGER li;
  long long tt = 0;
  GetSystemTimeAsFileTime(&ft);
  li.LowPart = ft.dwLowDateTime;
  li.HighPart = ft.dwHighDateTime;
  tt = (li.QuadPart - EPOCHFILETIME) / 10 / 1000;
  return (long long)tt;
#else
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
}

long long Date::nowEx() {
#ifdef _WIN32
  FILETIME ft;
  LARGE_INTEGER li;
  long long tt = 0;
  GetSystemTimeAsFileTime(&ft);
  li.LowPart = ft.dwLowDateTime;
  li.HighPart = ft.dwHighDateTime;
  tt = (li.QuadPart - EPOCHFILETIME) / 10;
  return tt;
#else
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (long long)tv.tv_sec * 1000 * 1000 + (long long)tv.tv_usec;
#endif
}
