#ifndef __JS_MATH_H__
#define __JS_MATH_H__
#include <random>

class Math {
 public:
  static double random();
  static int64_t floor(double);
  static int64_t ceil(double);
  static int64_t round(double);
 private:
  Math();
  static std::random_device _rd;
  static std::default_random_engine _e;
  static std::uniform_real_distribution<double> _u;
};

#endif
