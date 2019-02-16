#ifndef _ACB_TRACKLIST_H_
#define _ACB_TRACKLIST_H_

#include "./Reader.h"
#include "./UTFTable.h"

typedef struct {
  unsigned int cueId;
  std::string cueName;
  unsigned short wavId;
  unsigned char encodeType;
  unsigned char streaming;
} track;

class TrackList {
private:
  
public:
  UTFTable* cueTable;
  UTFTable* cueNameTable;
  UTFTable* waveformTable;
  UTFTable* synthTable;
  track* tracks;
  unsigned int length;

  TrackList(UTFTable* utf);
  ~TrackList();
};
#endif // !_ACB_TRACKLIST_H_
