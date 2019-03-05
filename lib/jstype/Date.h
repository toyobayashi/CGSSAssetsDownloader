#ifndef __DATE_H__
#define __DATE_H__

class Date {
private:
  Date();

public:
  static long long now();
  static long long nowEx();
};

#endif