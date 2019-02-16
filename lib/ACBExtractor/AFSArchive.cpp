#include "./include/AFSArchive.h"

void AFSArchive::readHeader() {
  std::streampos back = r->tell();
  r->seek(5);
  header = new awbHeader;

  header->offsetSize = r->readUInt8();
  r->seek(8);
  header->fileCount = r->readUInt32LE();
  header->alignment = r->readUInt32LE();
  header->ids = new unsigned short[header->fileCount];
  header->fileEndPoints = new unsigned int[header->fileCount + 1];

  for (unsigned int i = 0; i < header->fileCount; i++) {
    header->ids[i] = r->readUInt16LE();
  }

  for (unsigned int i = 0; i < header->fileCount + 1; i++) {
    header->fileEndPoints[i] = r->readUIntLE(header->offsetSize);
  }

  r->seek(back);
}

AFSArchive::AFSArchive(unsigned char * awb, unsigned int l) {
  r = new Reader(awb, l);
  r->seek(0);
  unsigned int t = r->readUInt32BE();
  if (t != 0x41465332) {
    r->close();
    delete r;
    throw "Not AWB file.";
  }
  
  length = l;
  readHeader();

  files = new entryFile[header->fileCount];
  for (unsigned int i = 0; i < header->fileCount; i++) {
    unsigned int tmp = header->fileEndPoints[i] % header->alignment;
    
    unsigned int start = (tmp == 0 ? header->fileEndPoints[i] / header->alignment : header->fileEndPoints[i] / header->alignment + 1) * header->alignment;
    unsigned int length = header->fileEndPoints[i + 1] - start;
    files[header->ids[i]].buf = new unsigned char[length];
    files[header->ids[i]].length = length;
    files[header->ids[i]].id = header->ids[i];
    
    std::streampos back = r->tell();
    r->seek(start);
    r->read((char*)files[header->ids[i]].buf, length);
    r->seek(back);
  }
  r->close();
}

AFSArchive::~AFSArchive() {
  delete r;
  for (unsigned int i = 0; i < header->fileCount; i++) {
    delete[] files[i].buf;
  }
  delete[] files;
  delete[] header->fileEndPoints;
  delete[] header->ids;
  delete header;
  
}
