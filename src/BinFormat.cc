#include "BinFormat.h"

#include <memory>
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
#define FRAME_MARKER "frame"

void read_FrameData(BinDataInfo_t* src1, BinDataInfo_t* src2,
                    BinDataInfo_t* temp) {
  std::unique_ptr<char[]> framedata1(new char[src1->header_Info.singleFrameSz]);
  std::unique_ptr<char[]> framedata2(new char[src2->header_Info.singleFrameSz]);

  BinFrameHead_t frameHead;
  uint16_t r = 0;
  for (size_t i = 0; i < src1->frameNum; i++) {
    uint64_t offs1 =
      src1->header_Info.headerSz + src1->header_Info.singleFrameSz * i;
    uint64_t offs2 =
      src2->header_Info.headerSz + src2->header_Info.singleFrameSz * i;
    fseeko(src1->filePtr, offs1, SEEK_SET);
    fseeko(src1->filePtr, offs2, SEEK_SET);

    strcpy(frameHead.frameMarker, FRAME_MARKER);
    frameHead.frameNum = i;
    sprintf(frameHead.str, "#%d", frameHead.frameNum);

    r = fwrite(&frameHead, sizeof(frameHead), 1, temp->filePtr);

    if (r != 1) {
      cerr << "Error reading frame header at " << __FILE__ << ":" << __LINE__
           << ": " << strerror(errno) << endl;
      exit(EXIT_FAILURE);
    }

    for (size_t j = 0; j < src1->header_Info.sensorCnt; j++) {
      r = fread(framedata1.get(), src1->header_Info.singleFrameSz, 1,
                src1->filePtr);
      r = fread(framedata2.get(), src2->header_Info.singleFrameSz, 1,
                src2->filePtr);

      if (r != 1) {
        cerr << "Error reading sensor data for sensor " << j << " at "
             << __FILE__ << ":" << __LINE__ << ": " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
      }

      r = fwrite(&framedata1[sizeof(frameHead)],
                 src1->header_Info.singleFrameSz - sizeof(frameHead),
                 1, temp->filePtr);
      r = fwrite(&framedata2[sizeof(frameHead)],
                 src2->header_Info.singleFrameSz - sizeof(frameHead),
                 1, temp->filePtr);
      // _write_frameL00(src1, src2, framedata1, framedata2);
    }
  }
}

// file 정보 읽기
// void read_BinFile(BinDataInfo_t* input_F) {
//   input_F->filePtr = fopen(input_F->filePath, "r");

//   read_BinHeader(input_F);
//   read_FrameData(input_F);
// }
