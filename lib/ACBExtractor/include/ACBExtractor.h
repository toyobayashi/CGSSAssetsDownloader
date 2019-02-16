#ifndef _ACB_ACBEXTRACTOR_H_
#define _ACB_ACBEXTRACTOR_H_

#include "./UTFTable.h"
#include "./AFSArchive.h"
#include "./TrackList.h"

class ACBExtractor {
private:

public:
  std::string path;
  UTFTable* headerTable;
  TrackList* tracklist;
  AFSArchive* awbFile;

  ACBExtractor(std::string acbFile);

  bool extract(void (*callback)(std::string filename, unsigned int length));

  ~ACBExtractor();
};
#endif // !_ACB_ACBEXTRACTOR_H_
