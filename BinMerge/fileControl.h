#ifndef _FILE_LOAD_H
#define _FILE_LOAD_H

#include "BinMerge.h"

typedef float float32_t;

typedef struct {
  uint32_t width;
  uint32_t height;
  uint32_t channels;
} ImgMeta;
typedef struct {
  char reserved[44];
  ImgMeta imgMeta;
} BinHeadReserved;

typedef struct {
  char marker[8];
  uint16_t majorVer;
  uint16_t minorVer;
  uint32_t headerSz;
  uint32_t sensorCnt;
  uint32_t calCnt;
  uint32_t singleFrameSz;
  char reserved[36 + 64];
  char str[128];
} BinHeaderInfo;

typedef struct {
  uint32_t sensorId;
  float32_t x;
  float32_t y;
  float32_t z;
  float32_t roll;
  float32_t pitch;
  float32_t yaw;
  char reserved[36];
  char str[64];
} BinHeadCal;

typedef struct {
  uint32_t sensorId;
  char subId[SENSOR_NAME_LEN];
  uint32_t frameSz;
  BinHeadReserved rsv;
  char str[64];
} BinHeadSensor;

typedef struct {
  char frameMarker[8];
  uint32_t frameNum;
  char str[12];
} BinFrameHead;

typedef struct {
  char frameMarker[8];
  uint32_t frameNum;
  uint64_t timelog;
  char str[12];
  char reserved[16];
} __attribute__((packed, aligned(1))) BinFrameHead2d0;

typedef struct {
  uint64_t timelog;
  char str[24];
} __attribute__((packed, aligned(1))) TimeStamp;

typedef struct {
  std::string filePath;
  std::fstream file;
  eBinMajorVersion majorVer;
  eBinMinorVersion minorVer;
  uint32_t lidarNum;
  uint32_t frameNum;
  uint64_t size;
  BinHeaderInfo binHead;
  std::unique_ptr<BinHeadSensor[]> sensors;
  std::unique_ptr<BinHeadCal[]> cal;
} BinFileInfo;

typedef struct {
  BinHeaderInfo binHead;
  std::unique_ptr<BinHeadSensor[]> sensors;
  std::unique_ptr<BinHeadCal[]> cal;
} FileHeader;

typedef struct {
  BinFrameHead2d0 frameHeader;
  TimeStamp time;
  std::unique_ptr<char[]> frameData;
} FileFrame2d1;
typedef struct {
  FileHeader fileHeader;
  std::unique_ptr<FileFrame2d1[]> frame;
} File;
namespace MergeLidarData {
class updata_File {
 private:
  BinFrameHead frameHeader;
  BinFrameHead2d0 frameHeader2d0;
  TimeStamp timeStamp;

 public:
  updata_File(){};
  std::unique_ptr<BinFileInfo[]> fileInfo;
  std::unique_ptr<File[]> inputFile;
  uint32_t mergeOption;
  uint32_t splitNum_start;
  uint32_t splitNum_end;
  std::unique_ptr<BinFileInfo> outPutFile;

  uint32_t checkOpt(int argc, std::vector<std::string> argv, uint32_t* option,
                    uint32_t splitStart, uint32_t splitEnd);
  void checkHeaderInfo(uint32_t fileIndex);
  bool read_File_Header(uint32_t fileIndex);
  void binSplit(uint32_t splitNum);
  bool merge_Header(uint32_t fileIndex);
  uint32_t calcFrameNum(uint32_t fileIndex);
  bool merge_Frame(uint32_t startFrmae, uint32_t endFrmae, uint32_t fileIndex);
  bool merge_Frame_Timecompare(uint32_t startFrmae, uint32_t endFrmae,
                               uint32_t fileIndex);
  uint32_t calcSingleframeSize(uint32_t fileIndex);
  uint32_t split(uint32_t splitNum_start, uint32_t splitNum_end);
  void insertFrame_2d1(uint32_t splitNum_start, uint32_t splitNum_end);
  void insertFrame_2d0(uint32_t splitNum_start, uint32_t splitNum_end);
  void insertFrame_1d2(uint32_t frameNum);
  void timcompare2d1(uint32_t frameNum, uint32_t fileIndex);
  void mergeFrame_2d1(uint32_t frameNum, uint32_t fileIndex);
  void mergeFrame_2d0(uint32_t frameNum, uint32_t fileIndex);
};

}  // namespace MergeLidarData

#endif
