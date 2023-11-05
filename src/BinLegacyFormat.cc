#include "BinLegacyFormat.h"

#include <stddef.h>
#include <unistd.h>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

using namespace std;

static const LegacyBin_t binLegacy[] = {
  [0] = {.cnt = 0,
         .ver = BIN_LEG_VER_0,
         .lidarCnt = 0,
         .imageCnt = 0,
         .radarCnt = 0},
  [1] = {.cnt = 10,
         .ver = BIN_LEG_VER_1,
         .lidarCnt = 3,
         .imageCnt = 1,
         .radarCnt = 1},
  [2] = {.cnt = 11,
         .ver = BIN_LEG_VER_2,
         .lidarCnt = 3,
         .imageCnt = 1,
         .radarCnt = 1},
  [3] = {.cnt = 14,
         .ver = BIN_LEG_VER_3,
         .lidarCnt = 3,
         .imageCnt = 4,
         .radarCnt = 1},
  [4] = {.cnt = 15,
         .ver = BIN_LEG_VER_4,
         .lidarCnt = 4,
         .imageCnt = 4,
         .radarCnt = 1},
  [5] = {.cnt = 18,
         .ver = BIN_LEG_VER_5,
         .lidarCnt = 5,
         .imageCnt = 4,
         .radarCnt = 2},
};

static bool isLidar(int senType) {
  if (senType == 0 || senType == 1 || senType == 2 || senType == 21 ||
      senType == 26 || senType == 27) {
    return true;
  }
  return false;
}

static int lidarSenTypeToIdx(int senType) {
  // clang-format off
  if      (senType == 0)  return 0;
  else if (senType == 1)  return 1;
  else if (senType == 2)  return 2;
  else if (senType == 21) return 3;
  else if (senType == 26) return 4;
  else if (senType == 27) return 5;
  else return 6;
  // clang-format on
}

const LegacyBin_t* getBinLegacyElementInfo(eLegacyBinVer ver) {
  if (ver < BIN_LEG_VER_1 || ver > BIN_LEG_VER_5) {
    return NULL;
  }
  return &binLegacy[ver];
}

void readLegacyBinLidarCal(ifstream& file, uint32_t lidarNum) {
  // skip cal data
  uint32_t itemNum = 5;  // x,y,z,angle,tilt
  LegacyBinHead_t tmp[lidarNum * itemNum];
  file.read((char*)tmp, sizeof(tmp));
  cout << " ================================ " << endl;
  cout << ARRAY_NUM(tmp) << endl;
  // file.seekg(sizeof(LegacyBinHead_t) * lidarNum * itemNum, ios::cur);
  for (size_t i = 0; i < ARRAY_NUM(tmp); i++) {
    printf("%s\n", tmp[i].markerStr);
  }
}

void readLegacyBinHeader(LegacyBinFileInfo_t* inf) {
  inf->file->read((char*)inf->header, sizeof(*inf->header) * inf->defInfo->cnt);
  for (size_t i = 0; i < inf->defInfo->cnt; i++) {
    printf("%s, ver: %d\n", inf->header[i].markerStr, inf->header[i].ver);
    if (strncmp(inf->header[i].markerStr, "ImageVersion",
                strlen("ImageVersion"))) {
      if (inf->header[i].ver > 1) {
        // don't care of overwriting data.
        inf->img.ver = inf->header[i].ver;
      }
    }
  }
}

int indexingLegacyBin(LegacyBinFileInfo_t* inf) {
  LegacyBinFrameStartInfo_t frameStart;
  inf->frameNum = 0;
  uint64_t pos = 0;
  ifstream* file = inf->file;
  inf->file->seekg(sizeof(LegacyBinHead_t) +
                     sizeof(LegacyBinHead_t) * inf->defInfo->cnt +
                     sizeof(LegacyBinHead_t) * inf->defInfo->lidarCnt * 5,
                   ios::beg);

  while (pos < inf->fileSz) {
    inf->frameIdx[inf->frameNum] = file->tellg();
    file->seekg(inf->frameIdx[inf->frameNum], ios::beg);
    file->read((char*)&frameStart, sizeof(frameStart));
    if (strcmp("FrameNumber", frameStart.delim)) {
      cout << "FrameNum: " << inf->frameNum - 1
           << " Unknown Delimiter: " << frameStart.delim << endl;
      exit(1);
    }
    inf->frameNum += 1;

    LegacyBinFrameMidInfo_t mid;
    LegacyBinFrameImageMidInfo_t imgMid;
    LegacyBinFrameEndInfo_t end;
    memset(&mid, 0, sizeof(mid));

    while (true) {
      uint64_t ofs = file->tellg();
      ofs += mid.size;
      if (ofs >= inf->fileSz) {
        pos = ofs;
        break;
      }

      file->seekg(ofs, ios::beg);
      file->read((char*)&end, sizeof(end));
      if (strcmp(end.delim, "FrameEnd") == 0) {
        if (inf->frameNum % 80 == 0) {
          cout << endl;
        }
        cout << "#";
        fflush(stdout);
        pos = ofs;
        break;
      } else {
        file->seekg(ofs, ios::beg);
      }
      file->read((char*)&mid, sizeof(mid));
      // 9 is image type in vueon project
      if (mid.sensorType == 9 && inf->img.ver > 1) {
        file->read((char*)&imgMid.width,
                   sizeof(imgMid) - sizeof(imgMid.midInfo));
        // don't care of overwriting data
        inf->img.height = imgMid.height;
        inf->img.width = imgMid.width;
      }

      if (isLidar(mid.sensorType)) {
        inf->sens.maxBufSz[lidarSenTypeToIdx(mid.sensorType)] =
          mid.size > inf->sens.maxBufSz[lidarSenTypeToIdx(mid.sensorType)]
            ? mid.size
            : inf->sens.maxBufSz[lidarSenTypeToIdx(mid.sensorType)];
      }
    }
  }
  cout << endl << "done" << endl;
  cout << "frame number: " << inf->frameNum << endl;
  return 0;
}

void setSensorIdFromString(char* str, LegacyBinFileInfo_t* inf) {
  string tmp = str;
  istringstream sns(tmp);
  string token;
  while (std::getline(sns, token, ',')) {
    inf->sens.id[inf->sens.cnt++] = (SensorId)stoi(token);
  }
}
