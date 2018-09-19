#ifndef __LZ4_H__
#define __LZ4_H__

#include <iostream>
#include <vector>

std::vector<uint8_t> readFile (std::string);

class BinaryReader {
private:
  std::vector<uint8_t> ary;
  int curPos;
public:
  BinaryReader (std::vector<uint8_t>&);
  int readByte ();
  int readShortLE ();
  int readIntLE ();
  std::vector<uint8_t>& copyBytes (std::vector<uint8_t>&, int&, int&);
  void seekAbs (int);
  void seekRel (int);
  int getPos ();
};

class LZ4Decompressor {
public:
  std::vector<uint8_t> decompress (std::vector<uint8_t>&);
  int readAdditionalSize (BinaryReader&);
};

void lz4dec (std::string, std::string);

#endif // !__LZ4_H__
