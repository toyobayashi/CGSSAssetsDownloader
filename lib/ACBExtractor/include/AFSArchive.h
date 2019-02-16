#ifndef _ACB_AFSARCHIVE_H_
#define _ACB_AFSARCHIVE_H_

#include "./Reader.h"

typedef struct {
  unsigned char offsetSize;
  unsigned int fileCount;
  unsigned int alignment;
  unsigned short* ids;
  unsigned int* fileEndPoints;
} awbHeader;

typedef struct {
  unsigned char* buf;
  unsigned int length;
  unsigned short id;
} entryFile;

class AFSArchive {
private:
  Reader* r;
  void readHeader();
public:

  unsigned int length;
  awbHeader* header;
  entryFile* files;
  
  AFSArchive(unsigned char* awb, unsigned int l);
  ~AFSArchive();
};
#endif // !_ACB_AFSARCHIVE_H_
