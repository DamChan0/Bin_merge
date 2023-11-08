#ifndef BIN_DATA_INFO_H
#define BIN_DATA_INFO_H
#include <time.h>

#include <iostream>
#include <stdexcept>

#include "type.h"
#define F_MAX_LEN           300
#define OPT_FMT             "f:n:l:s:"
#define SENSOR_NAME_LEN     32
#define LIDAR_SENSOR_ID_MIN 2
#define LIDAR_SENSOR_ID_MAX 37
#define F_NAME_LEN          260

using namespace std;

typedef char* string_t;

typedef struct {
  uint32_t width;
  uint32_t height;
  uint32_t channels;
} ImgMeta_t;

typedef union {
  char reserved[56];
  ImgMeta_t imgMeta;
} BinHeadReserved_u;

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
} __attribute__((packed, aligned(4))) BinHeadInfo_t;

typedef struct ver5_head_sensor {
  uint32_t sensorId;
  char subId[SENSOR_NAME_LEN];
  uint32_t frameSz;
  BinHeadReserved_u rsv;
  char str[64];
} __attribute__((packed, aligned(4))) BinHeadSensor_t;

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
} __attribute__((packed, aligned(4))) BinHeadCal_t;

typedef struct {
  char filePath[F_MAX_LEN];
  FILE* filePtr;
  BinHeadInfo_t header_Info;
  BinHeadSensor_t header_sensor[10];
  BinHeadCal_t header_cal[10];
  uint32_t frameNum;
  uint32_t file_size;
  uint32_t lidarCnt;
} BinDataInfo_t;

typedef struct {
  char frameMarker[8];
  uint32_t frameNum;
  char str[12];
} __attribute__((packed, aligned(1))) BinFrameHead_t;

void read_BinFile(BinDataInfo_t* file1);  // file 정보 읽기
void make_newFile(BinDataInfo_t* newfile, char* mergeFlag);

void init_Header(BinDataInfo_t* input_file);

void read_BinHeader(BinDataInfo_t* input_file);
void read_FrameData(BinDataInfo_t* src1, BinDataInfo_t* src2,
                    BinDataInfo_t* temp);
void write_HeaderInfo(BinDataInfo_t* input_file);
void write_SeneorHeader(BinDataInfo_t* input_file);
void write_CalInfo(BinDataInfo_t* input_file);
void write_FrameData(BinDataInfo_t* input_file);

bool isSensorId(uint32_t id);
#endif  // BIN_DATA_INFO_H