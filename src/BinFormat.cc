#include "BinFormat.h"

bool isSensorId(uint32_t id) {
  if (id < LIDAR_SENSOR_ID_MIN || id > LIDAR_SENSOR_ID_MAX) {
    return false;
  }
  return true;
}

void read_BinHeader(BinDataInfo_t* input_F) {
  fseeko(input_F->filePtr, 0, SEEK_END);
  input_F->file_size = ftello(input_F->filePtr);
  fseeko(input_F->filePtr, 0, SEEK_SET);
  uint16_t r = 0;
  // read header info
  r = fread(&input_F->header_Info, sizeof(BinHeadInfo_t), 1, input_F->filePtr);
  if (r != 1) {
    cerr << "Error reading header info at: " << __FILE__ << ":" << __LINE__
         << ": " << strerror(errno) << endl;
    exit(1);
  }
  input_F->frameNum = (input_F->file_size - input_F->header_Info.headerSz) /
                      input_F->header_Info.singleFrameSz;

  cout << "log major: " << input_F->header_Info.majorVer
       << ", minor: " << input_F->header_Info.minorVer
       << ", header sz: " << input_F->header_Info.headerSz
       << ", sensor cnt: " << input_F->header_Info.sensorCnt
       << ", cal cnt: " << input_F->header_Info.calCnt
       << ", whole frame sz: " << input_F->header_Info.singleFrameSz
       << "total frameNum : " << input_F->frameNum << "\n";

  // read sensor header
  for (uint32_t i = 0; i < input_F->header_Info.sensorCnt; i++) {
    r = fread(&input_F->header_sensor[i], sizeof(BinHeadSensor_t), 1,
              input_F->filePtr);
    if (r != 1) {
      cerr << "Error reading sensor header info at: " << __FILE__ << ":"
           << __LINE__ << ": " << strerror(errno) << std::endl;
      exit(1);
    }
    cout << "sensor id: " << input_F->header_sensor[i].sensorId
         << ", frame sz: " << input_F->header_sensor[i].frameSz << "\n";

    if (isSensorId(input_F->header_sensor[i].sensorId)) {
      input_F->lidarCnt++;
    }
  }

  // raed cal header
  for (uint32_t i = 0; i < input_F->header_Info.calCnt; i++) {
    fread(&input_F->header_cal[i], sizeof(BinHeadCal_t), 1, input_F->filePtr);
    cout << "cal id: " << input_F->header_cal[i].sensorId
         << ", x: " << input_F->header_cal[i].x
         << ", y: " << input_F->header_cal[i].y
         << ", z: " << input_F->header_cal[i].z
         << ", roll: " << input_F->header_cal[i].roll
         << ", pitch: " << input_F->header_cal[i].pitch
         << ", yaw: " << input_F->header_cal[i].yaw << endl;
  }
}

void read_FrameData(BinDataInfo_t* input_F, BinDataInfo_t* input_F2,
                    uint8_t* framedata1[], uint8_t* framedata2[]) {
  BinFrameHead_t frameHead;
  uint16_t r = 0;
  uint64_t headerOfs = input_F->header_Info.headerSz;
  fseeko(input_F->filePtr, headerOfs, SEEK_SET);

  for (size_t i = 0; i < input_F->header_Info.sensorCnt; i++) {
    framedata1[i] = (uint8_t*)malloc(input_F->header_sensor[i].frameSz);
  }

  for (size_t i = 0; i < input_F->frameNum; i++) {
    r = fread(&frameHead, sizeof(frameHead), 1, input_F->filePtr);
    if (r != 1) {
      cerr << "Error reading frame header at " << __FILE__ << ":" << __LINE__
           << ": " << strerror(errno) << endl;
      exit(EXIT_FAILURE);
    }
    for (size_t j = 0; j < input_F->header_Info.sensorCnt; j++) {
      r = fread(framedata1[j], input_F->header_sensor[j].frameSz, 1,
                input_F->filePtr);

      if (r != 1) {
        cerr << "Error reading sensor data for sensor " << j << " at "
             << __FILE__ << ":" << __LINE__ << ": " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
      }
      _write_frameL00(input_F, input_F2, framedata1, framedata2);
    }
  }
}

// file 정보 읽기
// void read_BinFile(BinDataInfo_t* input_F) {
//   input_F->filePtr = fopen(input_F->filePath, "r");

//   read_BinHeader(input_F);
//   read_FrameData(input_F);
// }
