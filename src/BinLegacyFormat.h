#ifndef __BIN_LEGACY_FMT_H__
#define __BIN_LEGACY_FMT_H__
#include <stddef.h>
#include <type.h>
#include <unistd.h>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#define BIN_LEG_SECT_NAME_SZ 64

using namespace std;

typedef enum {
  BIN_LEG_VER_0,
  BIN_LEG_VER_1,
  BIN_LEG_VER_2,
  BIN_LEG_VER_3,
  BIN_LEG_VER_4,
  BIN_LEG_VER_5,
  BIN_LEG_VER_MAX,
} eLegacyBinVer;

typedef struct {
  uint32_t cnt;
  eLegacyBinVer ver;
  uint32_t lidarCnt;
  uint32_t imageCnt;
  uint32_t radarCnt;
} LegacyBin_t;

#define MARKER_STR_SZ 128

typedef struct {
  char markerStr[MARKER_STR_SZ];
  int32_t ver;
} LegacyBinHead_t;

typedef struct {
  char delim[MARKER_STR_SZ];
  int32_t frameNum;
  int32_t frameSize;
} LegacyBinFrameStartInfo_t;

typedef struct {
  uint32_t sensorType;
  uint32_t timestamp;
  uint32_t size;
} LegacyBinFrameMidInfo_t;

typedef struct {
  LegacyBinFrameMidInfo_t midInfo;
  uint32_t width;
  uint32_t height;
} LegacyBinFrameImageMidInfo_t;

typedef struct {
  char delim[MARKER_STR_SZ];
  uint32_t dataReadTimeStamp;
  uint32_t processTimeStamp;
} LegacyBinFrameEndInfo_t;

typedef struct {
  ifstream* file;
  string filePath;
  uint64_t fileSz;
  eLegacyBinVer ver;
  struct {
    uint32_t width;
    uint32_t height;
    uint32_t ver;
  } img;
  const LegacyBin_t* defInfo;
  struct {
    uint32_t cnt;
    SensorId id[100];
    uint32_t maxBufSz[100];
  } sens;
  LegacyBinHead_t header[100];
  uint32_t frameNum;
  uint64_t frameIdx[100000];
} LegacyBinFileInfo_t;

const LegacyBin_t* getBinLegacyElementInfo(eLegacyBinVer ver);
void readLegacyBinHeader(LegacyBinFileInfo_t* inf);
int indexingLegacyBin(LegacyBinFileInfo_t* inf);
void setSensorIdFromString(char* str, LegacyBinFileInfo_t* inf);
void readLegacyBinLidarCal(ifstream& file, uint32_t lidarNum);

#endif  // __BIN_LEGACY_FMT_H__
