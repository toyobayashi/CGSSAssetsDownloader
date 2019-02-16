#include "./include/Reader.h"
#include <math.h>

int Reader::isLittleEndian = -1;

void Reader::checkEndian() {
  if (isLittleEndian == -1) {
    int x = 0x12;
    isLittleEndian = *(char*)&x == 0x12 ? 1 : 0;
  }
}

// template<typename T>
// T Reader::readByte(bool isLE) {
//   unsigned char buf[sizeof(T)];
//   unsigned int size = sizeof(T);
//   read((char*)buf, size);
//   if ((isLittleEndian && !isLE) || (!isLittleEndian && isLE)) {
//     unsigned char temp;
//     for (unsigned int i = 0; i < size / 2; i++) {
//       temp = buf[i];
//       buf[i] = buf[size - 1 - i];
//       buf[size - 1 - i] = temp;
//     }
//   }
  
//   T* result = (T*)buf;

//   return *result;
// }

Reader::Reader(unsigned char* buf, unsigned int length) {
  checkEndian();
  std::string tmpname = std::tmpnam(nullptr);
  tmpfile = tmpname;
  
  std::ofstream fs(tmpname, std::ios::binary);
  fs.write((const char*)buf, length);
  fs.close();
  length = length;
  open(tmpfile, std::ios::binary);
}

Reader::Reader() {
  checkEndian();
  std::ifstream();
}

Reader::~Reader() {
  if (tmpfile != "") remove(tmpfile.c_str());
}

Reader::Reader(std::string filename, ios_base::openmode mode) {
  checkEndian();
  open(filename.c_str(), mode);
}

void Reader::open(std::string filename, ios_base::openmode mode) {
  std::ifstream::open(filename.c_str(), mode);
  std::streampos current = tellg();
  seekg(0, std::ios::end);
  length = (unsigned int)tellg();
  seekg(current, std::ios::beg);
}

std::streampos Reader::tell() {
  return tellg();
}

Reader& Reader::seek(std::streampos at, std::streampos offset) {
  seekg(at + offset, std::ios::beg);
  return *this;
}

unsigned int Reader::readUInt32BE() {
  return readByte<unsigned int>();
}

unsigned int Reader::readUInt32LE() {
  return readByte<unsigned int>(true);
}

unsigned long long Reader::readUInt64BE() {
  return readByte<unsigned long long>();
}

unsigned long long Reader::readUInt64LE() {
  return readByte<unsigned long long>(true);
}

unsigned short Reader::readUInt16BE() {
  return readByte<unsigned short>();
}

unsigned short Reader::readUInt16LE() {
  return readByte<unsigned short>(true);
}

unsigned char Reader::readUInt8() {
  return readByte<unsigned char>();
}

char Reader::readInt8() {
  return readByte<char>();
}

short Reader::readInt16BE() {
  return readByte<short>();
}

short Reader::readInt16LE() {
  return readByte<short>(true);
}

int Reader::readInt32BE() {
  return readByte<int>();
}

int Reader::readInt32LE() {
  return readByte<int>(true);
}

long long Reader::readInt64BE() {
  return readByte<int>();
}

long long Reader::readInt64LE() {
  return readByte<long long>(true);
}

double Reader::readDoubleBE() {
  return readByte<double>();
}

double Reader::readDoubleLE() {
  return readByte<double>(true);
}

float Reader::readFloatBE() {
  return readByte<float>();
}

float Reader::readFloatLE() {
  return readByte<float>(true);
}

unsigned int Reader::readUIntLE(unsigned int size) {
  unsigned char* buf = new unsigned char[size];

  read((char*)buf, size);

  unsigned int result = 0;
  for (unsigned int i = 0; i < size; i++) {
    int high, low, power;
    high = buf[i] / 16;
    low = buf[i] % 16;
    power = 2 * i; // 2 * size - 1 - 2 * i;
    result += (high * (unsigned int)pow(16, power + 1) + low * (unsigned int)pow(16, power));
  }

  delete[] buf;
  return result;
}
