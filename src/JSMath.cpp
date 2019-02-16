#include "JSMath.h"
#include <cmath>

std::random_device Math::_rd;
std::default_random_engine Math::_e(Math::_rd());
std::uniform_real_distribution<double> Math::_u(0, 1);

double Math::random() {
  return _u(_e);
}

int Math::floorA(double value) {
  return static_cast<int>(floor(value));
}
