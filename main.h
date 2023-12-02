#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#define UNUSED(x)  (void)(x)
#define ARR_NUM(x) (sizeof(x) / sizeof(*x))

#define OPT_FMT      "1:2:fd"
#define HD_MARKER    "vueron"
#define FRAME_MARKER "frame"
// ANSI Escape codes for text colors
#define ANSI_COLOR_RED    "\033[91m"
#define ANSI_COLOR_RESET  "\033[0m"
#define ANSI_COLOR_PURPLE "\033[95m"
#define ANSI_COLOR_GREEN  "\033[92m"
#define ANSI_COLOR_WHITE  "\033[47m"
#define BOLD              "\033[1m"

#define SENSOR_NAME_LEN 32
typedef float float32_t;
using namespace std;
typedef enum {
  BIN_LEG_VER_0,
  BIN_LEG_VER_1,
  BIN_LEG_VER_2,
  BIN_LEG_VER_3,
  BIN_LEG_VER_4,
  BIN_LEG_VER_5,
  BIN_LEG_VER_MAX,
} eBinLegVersion;

typedef enum {
  BIN_MAJOR_VER_NONE,
  BIN_MAJOR_VER_1,
  BIN_MAJOR_VER_2,
  BIN_MAJOR_VER_MAX,
} eBinMajorVersion;

typedef enum {
  BIN_MINOR_VER_NONE,
  BIN_MINOR_VER_1,
  BIN_MINOR_VER_2,
  BIN_MINOR_VER_MAX,
} eBinMinorVersion;

#define CUR_BIN_MAJOR  BIN_MAJOR_VER_2
#define CUR_BIN_MINOR  BIN_MINOR_VER_NONE
#define NOT_LEGACY_BIN 0

typedef struct {
  uint32_t width;
  uint32_t height;
  uint32_t channels;
} ImgMeta_t;

typedef union {
  char reserved[56];
  ImgMeta_t imgMeta;
} BinHeadReserved_u;

typedef struct ver5_head_sensor {
  uint32_t sensorId;
  char subId[SENSOR_NAME_LEN];
  uint32_t frameSz;
  BinHeadReserved_u rsv;
  char str[64];
} __attribute__((packed, aligned(1))) BinHeadSensor_t;

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
} __attribute__((packed, aligned(1))) BinHeadCal_t;

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
} __attribute__((packed, aligned(1))) BinHeadInfo_t;

typedef struct {
  string fileName;
  std::fstream file;
  BinHeadInfo_t binHead;
  BinHeadSensor_t sensors[10];
  BinHeadCal_t cal[10];
  uint32_t frameNum;
  uint64_t size;
  uint32_t lidarNum;
  eBinMajorVersion majorVer;
} BinFileInfo_t;

typedef struct {
  char frameMarker[8];
  uint32_t frameNum;
  char str[12];
} __attribute__((packed, aligned(1))) BinFrameHead_t;

typedef struct {
  char frameMarker[8];
  uint32_t frameNum;
  uint64_t timelog;
  char str[12];
  char reserved[16];
} __attribute__((packed, aligned(4))) BinFrameHead2d0_t;

typedef struct {
  uint64_t timelog;
  char str[24];
} __attribute__((packed, aligned(4))) TimeStamp_t;

typedef struct {
  char frameMarker[8];
  uint32_t frameNum;
  char reserved[20];
} __attribute__((packed, aligned(4))) BinFrameHead2d1_t;

void check_file_length(std::unique_ptr<char[]>& frameHeader_Buffer1,
                       std::unique_ptr<char[]>& frameHeader_Buffer2,
                       BinFileInfo_t& src1, BinFileInfo_t& src2);
void printWarning(const std::string& message);
void printTime(uint32_t sec, uint32_t min);
void printError(const std::string& message);
void printCorrect(const std::string& message);
void printDebug(const std::string& message, const std::string& flag);
int mergeBinFrame_time(BinFileInfo_t& src1, BinFileInfo_t& src2,
                       BinFileInfo_t& dst);
void set_and_print_outputfile(BinFileInfo_t& output, int frameNum);
int checkOpt(string& file1, string& file2, int argc, char* argv[]);
int loadBinHeader(BinFileInfo_t& inf);
void checkBinVersion(BinFileInfo_t& file);
size_t getFrameHeadSzUsingVersion(int ver1, int ver2);

int mergeBinHeader(BinFileInfo_t& src1, BinFileInfo_t& src2,
                   BinFileInfo_t& dst);
int mergeBinFrame11(BinFileInfo_t& src1, BinFileInfo_t& src2,
                    BinFileInfo_t& dst);
int mergeBinFrame12(BinFileInfo_t& src1, BinFileInfo_t& src2,
                    BinFileInfo_t& dst);
void is_firstVer2(std::unique_ptr<char[]>& dataBuffer1,
                  std::unique_ptr<char[]>& dataBuffer2,
                  std::unique_ptr<char[]>& frameHeader_Buffer1,
                  BinFileInfo_t& src1, BinFileInfo_t& src2, BinFileInfo_t& dst,
                  int frameNum);
void is_secondVer2(std::unique_ptr<char[]>& dataBuffer1,
                   std::unique_ptr<char[]>& dataBuffer2,
                   std::unique_ptr<char[]>& frameHeader_Buffer1,
                   BinFileInfo_t& src1, BinFileInfo_t& src2, BinFileInfo_t& dst,
                   int frameNum);
int mergeBinFrame(BinFileInfo_t& src1, BinFileInfo_t& src2, BinFileInfo_t& dst);
int merge_possibility(int start1, int start2, int end1, int end2);
void check_file_length(std::unique_ptr<char[]>& frameHeader_Buffer1,
                       std::unique_ptr<char[]>& frameHeader_Buffer2,
                       BinFileInfo_t& src1, BinFileInfo_t& src2);
int mergeBinFrame_timecompare(BinFileInfo_t& src1, BinFileInfo_t& src2,
                              BinFileInfo_t& dst);
int mergeBinFrame_timecompare_force(BinFileInfo_t& src1, BinFileInfo_t& src2,
                                    BinFileInfo_t& dst);
void getPath(string& file1, string& file2);
int mergebin();
int mergeFrame2d1_timecomapre(BinFileInfo_t& src1, BinFileInfo_t& src2,
                              BinFileInfo_t& dst);
int mergeFrame2d1_force(BinFileInfo_t& src1, BinFileInfo_t& src2,
                        BinFileInfo_t& dst);

#endif  // __MAIN_H__