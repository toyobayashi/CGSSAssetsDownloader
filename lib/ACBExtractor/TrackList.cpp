#include "./include/TrackList.h"

TrackList::TrackList(UTFTable* utf) {
  acbData* cueTableData = utf->get(0, "CueTable");
  unsigned char* buf1 = new unsigned char[cueTableData->length];
  utf->readBinary(buf1, static_cast<unsigned int*>(cueTableData->data)[0], static_cast<unsigned int*>(cueTableData->data)[1]);
  cueTable = new UTFTable(buf1, cueTableData->length);
  delete[] buf1;

  acbData* cueNameTableData = utf->get(0, "CueNameTable");
  unsigned char* buf2 = new unsigned char[cueNameTableData->length];
  utf->readBinary(buf2, static_cast<unsigned int*>(cueNameTableData->data)[0], static_cast<unsigned int*>(cueNameTableData->data)[1]);
  cueNameTable = new UTFTable(buf2, cueNameTableData->length);
  delete[] buf2;

  acbData* waveformTableData = utf->get(0, "WaveformTable");
  unsigned char* buf3 = new unsigned char[waveformTableData->length];
  utf->readBinary(buf3, static_cast<unsigned int*>(waveformTableData->data)[0], static_cast<unsigned int*>(waveformTableData->data)[1]);
  waveformTable = new UTFTable(buf3, waveformTableData->length);
  delete[] buf3;

  acbData* synthTableData = utf->get(0, "SynthTable");
  unsigned char* buf4 = new unsigned char[synthTableData->length];
  utf->readBinary(buf4, static_cast<unsigned int*>(synthTableData->data)[0], static_cast<unsigned int*>(synthTableData->data)[1]);
  synthTable = new UTFTable(buf4, synthTableData->length);
  delete[] buf4;

  std::string* nameMap = new std::string[cueNameTable->header->rowLength];

  for (unsigned int i = 0; i < cueNameTable->header->rowLength; i++) {
    acbData* str = cueNameTable->get(i, "CueName");
    nameMap[*(static_cast<unsigned short*>(cueNameTable->get(i, "CueIndex")->data))] = str ?
      std::string(static_cast<char*>(str->data)) : "UNKNOWN";
  }

  tracks = new track[cueTable->header->rowLength];
  length = cueTable->header->rowLength;
  for (unsigned int i = 0; i < cueTable->header->rowLength; i++) {
    unsigned char referenceType = *(static_cast<unsigned char*>(cueTable->get(i, "ReferenceType")->data));
    if (referenceType != static_cast<unsigned char>(3) && referenceType != static_cast<unsigned char>(8)) {
      delete cueTable;
      delete cueNameTable;
      delete waveformTable;
      delete synthTable;
      delete[] tracks;
      throw "ReferenceType not implemented.";
    }

    unsigned short referenceIndex = *(static_cast<unsigned short*>(cueTable->get(i, "ReferenceIndex")->data));

    acbData* refitm = synthTable->get(referenceIndex, "ReferenceItems");
    unsigned char* referenceItems = new unsigned char[refitm->length];
    synthTable->readBinary(referenceItems, static_cast<unsigned int*>(refitm->data)[0], static_cast<unsigned int*>(refitm->data)[1]);
    unsigned short b = (referenceItems[2] / 16) * 4096 + (referenceItems[2] % 16) * 256 + (referenceItems[3] / 16) * 16 + (referenceItems[3] % 16);
    delete[] referenceItems;

    unsigned short wavId;
    acbData* hasId = waveformTable->get(b, "Id");
    if (hasId) {
      wavId = *(static_cast<unsigned short*>(hasId->data));
    } else {
      wavId = *(static_cast<unsigned short*>(waveformTable->get(b, "MemoryAwbId")->data));
    }
    
    tracks[i].cueId = *(static_cast<unsigned int*>(cueTable->get(i, "CueId")->data));
    tracks[i].cueName = nameMap[i];
    tracks[i].wavId = wavId;
    tracks[i].encodeType = *(static_cast<unsigned char*>(waveformTable->get(b, "EncodeType")->data));
    tracks[i].streaming = *(static_cast<unsigned char*>(waveformTable->get(b, "Streaming")->data));
  }

  delete[] nameMap;
}

TrackList::~TrackList() {
  delete cueTable;
  delete cueNameTable;
  delete waveformTable;
  delete synthTable;
  delete[] tracks;
}
