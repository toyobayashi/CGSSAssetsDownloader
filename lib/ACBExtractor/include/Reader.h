#ifndef _ACB_READER_H_
#define _ACB_READER_H_

#include <fstream>

class Reader : public std::ifstream {
private:
  
  static int isLittleEndian;
  void checkEndian();
  std::string tmpfile = "";

public:
  unsigned int length;
  Reader(unsigned char* buf, unsigned int length);
  Reader();
  ~Reader();
  Reader(std::string filename, ios_base::openmode mode);
  void open(std::string filename, ios_base::openmode mode);
  std::streampos tell();
  Reader& seek(std::streampos at, std::streampos offset = 0);
  unsigned int readUInt32BE();
  unsigned int readUInt32LE();
  unsigned long long readUInt64BE();
  unsigned long long readUInt64LE();
  unsigned short readUInt16BE();
  unsigned short readUInt16LE();
  unsigned char readUInt8();
  char readInt8();
  short readInt16BE();
  short readInt16LE();
  int readInt32BE();
  int readInt32LE();
  long long readInt64BE();
  long long readInt64LE();
  double readDoubleBE();
  double readDoubleLE();
  float readFloatBE();
  float readFloatLE();
  unsigned int readUIntLE(unsigned int size);

  template<typename T>
  T readByte(bool isLE = false) {
    unsigned char buf[sizeof(T)];
    unsigned int size = sizeof(T);
    read((char*)buf, size);
    if ((isLittleEndian && !isLE) || (!isLittleEndian && isLE)) {
      unsigned char temp;
      for (unsigned int i = 0; i < size / 2; i++) {
        temp = buf[i];
        buf[i] = buf[size - 1 - i];
        buf[size - 1 - i] = temp;
      }
    }
    
    T* result = (T*)buf;

    return *result;
  }
};
#endif // !_ACB_READER_H_
