#include "./include/ACBExtractor.h"
#include <regex>

ACBExtractor::ACBExtractor(std::string acbFile) {
  std::regex re("/|\\\\");
#ifdef _WIN32
  acbFile = std::regex_replace(acbFile, re, "\\");
#else
  std::regex_replace(acbFile, re, "/");
#endif
  path = acbFile;
  headerTable = new UTFTable(acbFile);
  tracklist = new TrackList(headerTable);

  acbData* awbData = headerTable->get(0, "AwbFile");
  unsigned char* awbbuf = new unsigned char[awbData->length];
  headerTable->readBinary(awbbuf, static_cast<unsigned int*>(awbData->data)[0], static_cast<unsigned int*>(awbData->data)[1]);
  awbFile = new AFSArchive(awbbuf, awbData->length);
  delete[] awbbuf;
}

bool ACBExtractor::extract(void (*callback)(std::string filename, unsigned int length)) {
  std::string dirname, targetDir, filename;

#ifdef _WIN32
  auto posw = path.find_last_of('\\');
  if (posw == std::string::npos) {
    dirname = ".";
    filename = path;
  } else {
    dirname = path.substr(0, posw);
    filename = path.substr(posw + 1);
  }
  targetDir = dirname + "\\_acb_" + filename;
  std::string cmd = "mkdir " + targetDir;
#else
  auto posl = path.find_last_of('/');
  if (posl == std::string::npos) {
    dirname = ".";
    filename = path;
  } else {
    dirname = path.substr(0, posl);
    filename = path.substr(posl + 1);
  }
  targetDir = dirname + "/_acb_" + filename;
  std::string cmd = "mkdir -p " + targetDir;
#endif
  
  system(cmd.c_str());
  std::ofstream fs;
  for (unsigned int i = 0; i < tracklist->length; i++) {
    int inFiles = -1;
    for (unsigned int j = 0; j < awbFile->header->fileCount; j++) {
      if (tracklist->tracks[i].wavId == awbFile->files[j].id) {
        inFiles = j;
        break;
      }
    }
    if (inFiles != -1) {
      std::string cueName = tracklist->tracks[i].cueName;
      std::string encodeType = "";
      switch (tracklist->tracks[i].encodeType) {
      case 0: {
        encodeType = ".adx";
        break;
      }
      case 2: {
        encodeType = ".hca";
        break;
      }
      case 7: {
        encodeType = ".at3";
        break;
      }
      case 8: {
        encodeType = ".vag";
        break;
      }
      case 9: {
        encodeType = ".bcwav";
        break;
      }
      case 13: {
        encodeType = ".dsp";
        break;
      }
      default:
        break;
      }
#ifdef _WIN32
      std::string acbFilename = targetDir + "\\" + cueName + encodeType;
#else
      std::string acbFilename = targetDir + "/" + cueName + encodeType;
#endif
      fs.open(acbFilename, std::ios::binary);
      fs.write((const char*)awbFile->files[inFiles].buf, awbFile->files[inFiles].length);
      if (callback) callback(cueName + encodeType, awbFile->files[inFiles].length);
      fs.close();
    }
    else {
      return false; 
    }
  }
  return true;
}

ACBExtractor::~ACBExtractor() {
  delete headerTable;
  delete tracklist;
  delete awbFile;
}
