#include "makefile.h"
extern BinDataInfo_t merged_File;  // 실제 쓰여질 파일 정보
extern uint8_t framedata;
void make_fileL00(BinDataInfo_t* file1, BinDataInfo_t* file2,
                  BinDataInfo_t* mergefile) {
  char new_FileName[300];
  std::time_t rawtime;
  struct std::tm* timeinfo;
  std::time(&rawtime);
  timeinfo = std::localtime(&rawtime);

  std::sprintf(new_FileName, "merge_%d_%d_%d_%d_%d_%d.bin",
               timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,
               timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min,
               timeinfo->tm_sec);

  std::cout << "File name: " << new_FileName << std::endl;

  std::ofstream outputFile(new_FileName, std::ios::binary);
  memcpy(mergefile->filePath, &new_FileName, sizeof(new_FileName));
  mergefile->filePtr = fopen(mergefile->filePath, "wb+");
  if (outputFile) {
    // Write your data to 'outputFile' here
    // For example, outputFile.write(data, dataSize);
    outputFile.close();

  } else {
    std::cerr << "Failed to create the file." << std::endl;
  }
}
void make_fileL11(LegacyBinFileInfo_t* file1, LegacyBinFileInfo_t* file2,
                  BinDataInfo_t* mergefile) {
  char new_FileName[256];
  std::time_t rawtime;
  struct std::tm* timeinfo;
  std::time(&rawtime);
  timeinfo = std::localtime(&rawtime);

  std::sprintf(new_FileName, "merge_%d_%d_%d_%d_%d_%d.bin",
               timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,
               timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min,
               timeinfo->tm_sec);

  std::cout << "File name: " << new_FileName << std::endl;
  std::ofstream outputFile(new_FileName, std::ios::binary);
  memcpy(mergefile->filePath, &new_FileName, sizeof(new_FileName));
  mergefile->filePtr = fopen(mergefile->filePath, "wb+");
  if (outputFile) {
    // Write your data to 'outputFile' here
    // For example, outputFile.write(data, dataSize);
    outputFile.close();

  } else {
    std::cerr << "Failed to create the file." << std::endl;
  }
}
void make_fileL01(BinDataInfo_t* file1, LegacyBinFileInfo_t* file2,
                  BinDataInfo_t* mergefile) {
  char new_FileName[256];
  std::time_t rawtime;
  struct std::tm* timeinfo;
  std::time(&rawtime);
  timeinfo = std::localtime(&rawtime);

  std::sprintf(new_FileName, "merge_%d_%d_%d_%d_%d_%d.bin",
               timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,
               timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min,
               timeinfo->tm_sec);

  std::cout << "File name: " << new_FileName << std::endl;
  memcpy(mergefile->filePath, &new_FileName, sizeof(new_FileName));
  mergefile->filePtr = fopen(mergefile->filePath, "wb+");
  std::ofstream outputFile(new_FileName, std::ios::binary);
  if (outputFile) {
    // Write your data to 'outputFile' here
    // For example, outputFile.write(data, dataSize);
    outputFile.close();

  } else {
    std::cerr << "Failed to create the file." << std::endl;
  }
}
void make_fileL10(LegacyBinFileInfo_t* file1, BinDataInfo_t* file2,
                  BinDataInfo_t* mergefile) {
  char new_FileName[256];
  std::time_t rawtime;
  struct std::tm* timeinfo;
  std::time(&rawtime);
  memcpy(mergefile->filePath, &new_FileName, sizeof(new_FileName));
  mergefile->filePtr = fopen(mergefile->filePath, "wb+");
  timeinfo = std::localtime(&rawtime);

  std::sprintf(new_FileName, "merge_%d_%d_%d_%d_%d_%d.bin",
               timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,
               timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min,
               timeinfo->tm_sec);

  std::cout << "File name: " << new_FileName << std::endl;

  std::ofstream outputFile(new_FileName, std::ios::binary);
  if (outputFile) {
    // Write your data to 'outputFile' here
    // For example, outputFile.write(data, dataSize);
    outputFile.close();

  } else {
    std::cerr << "Failed to create the file." << std::endl;
  }
}
void write_headerL00(BinDataInfo_t* file1, BinDataInfo_t* file2,
                     BinDataInfo_t* mergefile) {
  strcpy(mergefile->header_Info.marker, "vueron");

  BinHeadInfo_t* Hinfo = &mergefile->header_Info;
  BinHeadInfo_t* finfo1 = &file1->header_Info;
  BinHeadInfo_t* finfo2 = &file2->header_Info;
  Hinfo->majorVer = file1->header_Info.majorVer;
  Hinfo->minorVer = file1->header_Info.minorVer;
  Hinfo->calCnt = finfo1->calCnt + finfo2->calCnt;
  Hinfo->sensorCnt = finfo1->sensorCnt + finfo2->sensorCnt;
  mergefile->lidarCnt = file1->lidarCnt + file2->lidarCnt;
  Hinfo->headerSz = sizeof(BinHeadInfo_t) +
                    (sizeof(BinHeadSensor_t) * finfo1->sensorCnt) +
                    (sizeof(BinHeadCal_t) * finfo1->calCnt) +
                    (sizeof(BinHeadSensor_t) * finfo2->sensorCnt) +
                    (sizeof(BinHeadCal_t) * finfo2->calCnt);
  Hinfo->singleFrameSz = sizeof(BinFrameHead_t);
  for (size_t i = 0; i < Hinfo->sensorCnt; i++) {
    if (i < mergefile->lidarCnt) {
      Hinfo->singleFrameSz +=
        (file1->header_sensor[i].frameSz + file2->header_sensor[i].frameSz);
    } else {
      Hinfo->singleFrameSz +=
        (file1->header_sensor[i - file1->lidarCnt].frameSz +
         file2->header_sensor[i - file2->lidarCnt].frameSz);
    }
  }
  if (file1->frameNum <= file2->frameNum) {
    mergefile->frameNum = file1->frameNum;
  } else {
    mergefile->frameNum = file2->frameNum;
  }
  sprintf(Hinfo->str,
          "major: %d, minor: %d, header size: %d, sensor cnt: %d, cal cnt: %d "
          "single frame "
          "size: %u",
          Hinfo->majorVer, Hinfo->minorVer, Hinfo->headerSz, Hinfo->sensorCnt,
          Hinfo->calCnt, Hinfo->singleFrameSz);

  // write  header
  /************************************************************************/
  size_t r = fwrite(&mergefile->header_Info, sizeof(BinHeadInfo_t), 1,
                    mergefile->filePtr);
  fflush(mergefile->filePtr);

  if (!r) {
    std::cerr << "fail!! header_Info write at: " << __FILE__ << ":" << __LINE__
              << ": " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  } else {
    std::clog << "success !! header_Info write" << endl;
  }
  /************************************************************************/
  // write sensor header
  for (size_t i = 0; i < file1->header_Info.sensorCnt; i++) {
    BinHeadSensor_t* sheader = &mergefile->header_sensor[i];
    if (isSensorId(file1->header_sensor[i].sensorId)) {
      memcpy(&mergefile->header_sensor[i], &file1->header_sensor[i],
             sizeof(BinHeadSensor_t));
      r = fwrite(&mergefile->header_sensor[i], sizeof(BinHeadSensor_t), 1,
                 mergefile->filePtr);
      fflush(mergefile->filePtr);

    } else {
      mergefile->header_sensor[i].sensorId = 0;
      std::strncpy(mergefile->header_sensor[i].subId, "not_lidar",
                   SENSOR_NAME_LEN);

      r = fwrite(&mergefile->header_sensor[i], sizeof(BinHeadSensor_t), 1,
                 mergefile->filePtr);
      fflush(mergefile->filePtr);
    }
  }
  for (size_t i = file1->header_Info.sensorCnt;
       i < file1->header_Info.sensorCnt + file2->header_Info.sensorCnt; i++) {
    BinHeadSensor_t* sheader = &mergefile->header_sensor[i];

    if (isSensorId(
          file2->header_sensor[i - file1->header_Info.sensorCnt].sensorId)) {
      memcpy(&mergefile->header_sensor[i],
             &file2->header_sensor[i - file1->header_Info.sensorCnt],
             sizeof(BinHeadSensor_t));
      r = fwrite(&mergefile->header_sensor[i], sizeof(BinHeadSensor_t), 1,
                 mergefile->filePtr);
      fflush(mergefile->filePtr);

    } else {
      mergefile->header_sensor[i].sensorId = 0;
      std::strncpy(mergefile->header_sensor[i].subId, "not_lidar",
                   SENSOR_NAME_LEN);

      r = fwrite(&mergefile->header_sensor[i], sizeof(BinHeadSensor_t), 1,
                 mergefile->filePtr);
      fflush(mergefile->filePtr);
    }
  }
  if (!r) {
    std::cerr << "fail!! header_sensor write at: " << __FILE__ << ":"
              << __LINE__ << ": " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  } else {
    std::clog << "success !! header_sensor write" << endl;
  }
  /************************************************************************/
  // read cal header
  for (size_t i = 0; i < file1->lidarCnt; i++) {
    BinHeadCal_t* cheader = &mergefile->header_cal[i];
    memcpy(&mergefile->header_cal[i], &file1->header_cal[i],
           sizeof(BinHeadCal_t));
    sprintf(cheader->str, "calibration - id: #%d",
            mergefile->header_sensor[i].sensorId);

    r = fwrite(&mergefile->header_cal[i], sizeof(BinHeadCal_t), 1,
               mergefile->filePtr);

    fflush(mergefile->filePtr);
  }
  for (size_t i = file1->lidarCnt; i < file1->lidarCnt + file2->lidarCnt; i++) {
    BinHeadCal_t* cheader = &mergefile->header_cal[i];
    memcpy(&mergefile->header_cal[i], &file2->header_cal[i - file1->lidarCnt],
           sizeof(BinHeadCal_t));
    sprintf(cheader->str, "calibration - id: #%d",
            mergefile->header_sensor[i].sensorId);

    r = fwrite(&mergefile->header_cal[i], sizeof(BinHeadCal_t), 1,
               mergefile->filePtr);
    fflush(mergefile->filePtr);
  }

  // write cal header

  if (!r) {
    std::cerr << "fail!! header_cal write at: " << __FILE__ << ":" << __LINE__
              << ": " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  } else {
    std::clog << "success !! header_cal write" << endl;
  }
}
void write_headerL11(LegacyBinFileInfo_t* file1, LegacyBinFileInfo_t* file2,
                     BinDataInfo_t* convertBinFileInf1,
                     BinDataInfo_t* convertBinFileInf2,
                     BinDataInfo_t* mergefile) {
  ;
}
void write_headerL01(BinDataInfo_t* file1, LegacyBinFileInfo_t* file2,
                     BinDataInfo_t* convertBinFileInf1,
                     BinDataInfo_t* mergefile) {
  ;
}
void write_headerL10(LegacyBinFileInfo_t* file1, BinDataInfo_t* file2,
                     BinDataInfo_t* convertBinFileInf1,
                     BinDataInfo_t* mergefile) {
  ;
}
void _write_frameL00(BinDataInfo_t* input_F, BinDataInfo_t* input_F2,
                     uint8_t* framedata1, uint8_t* framedata2) {
  write_frameL00(input_F, input_F2, merged_File, framedata1, framedata2);
  memset(data1)
}

void write_frameL00(BinDataInfo_t* file1, BinDataInfo_t* file2,
                    BinDataInfo_t* mergefile, uint8_t* data1[],
                    uint8_t* data2[]) {
  BinFrameHead_t frameHead;
  strcpy(frameHead.frameMarker, (char*)"frame");

  uint32_t headerOfs = mergefile->header_Info.headerSz;
  fseek(mergefile->filePtr, headerOfs, SEEK_SET);

  for (int i = 0; i < mergefile->frameNum; i++) {
    frameHead.frameNum++;
    sprintf(frameHead.str, "#%d", frameHead.frameNum);
    fwrite(&frameHead, sizeof(frameHead), 1, mergefile->filePtr);
    fflush(mergefile->filePtr);

    for (size_t j = 0; j < file1->header_Info.sensorCnt; j++) {
      fwrite(data1[j], file1->header_sensor[j].frameSz, 1, mergefile->filePtr);
    }
    for (size_t j = 0; j < file2->header_Info.sensorCnt; j++) {
      fwrite(data2[j], file2->header_sensor[j].frameSz, 1, mergefile->filePtr);
    }
    fflush(mergefile->filePtr);
  }

  // else {
  //   frameHead.frameNum++;
  //   fseeko(file1->filePtr,
  //          file1->header_Info.headerSz +
  //            (file1->header_Info.singleFrameSz * i) + sizeof(frameHead),
  //          SEEK_SET);
  //   fseeko(file2->filePtr,
  //          file2->header_Info.headerSz +
  //            (file2->header_Info.singleFrameSz * i) + sizeof(frameHead),
  //          SEEK_SET);

  //   fread(&frameHead, sizeof(frameHead), 1, file2->filePtr);
  //   fseeko(file2->filePtr, sizeof(frameHead), SEEK_CUR);

  //   sprintf(frameHead.str, "#%d", frameHead.frameNum);
  //   fwrite(&frameHead, sizeof(frameHead), 1, mergefile->filePtr);

  //   for (size_t j = 0; j < file1->header_Info.sensorCnt; j++) {
  //     fwrite(file1->filePtr, file1->header_sensor->frameSz, 1,
  //            mergefile->filePtr);
  //     fseek(file1->filePtr, file1->header_Info.singleFrameSz, SEEK_CUR);
  //     fflush(mergefile->filePtr);
  //   }
  //   for (size_t j = 0; j < file2->header_Info.sensorCnt; j++) {
  //     fwrite(file2->filePtr, file2->header_sensor->frameSz, 1,
  //            mergefile->filePtr);
  //     fseek(file2->filePtr, file2->header_Info.singleFrameSz, SEEK_CUR);
  //     fflush(mergefile->filePtr);
  //   }
  // }

  std::ifstream fileToGetSize(mergefile->filePath);
  if (fileToGetSize) {
    fileToGetSize.seekg(0, std::ios::end);
    std::streampos fileSize = fileToGetSize.tellg();
    fileToGetSize.close();

    std::cout << "File created successfully. File size: " << fileSize
              << " bytes." << std::endl;
  } else {
    std::cerr << "Failed to get the file size." << std::endl;
  }
}
void write_frameL11(LegacyBinFileInfo_t* file1, LegacyBinFileInfo_t* file2,
                    BinDataInfo_t* convertBinFileInf1,
                    BinDataInfo_t* convertBinFileInf2,
                    BinDataInfo_t* mergefile) {
  ;
}
void write_frameL01(BinDataInfo_t* file1, LegacyBinFileInfo_t* file2,
                    BinDataInfo_t* convertBinFileInf1,
                    BinDataInfo_t* mergefile) {
  ;
}
void write_frameL10(LegacyBinFileInfo_t* file1, BinDataInfo_t* file2,
                    BinDataInfo_t* convertBinFileInf1,
                    BinDataInfo_t* mergefile) {
  ;
}

void merge_BinFile_L00(BinDataInfo_t* temp_File1, BinDataInfo_t* temp_File2,
                       BinDataInfo_t* mergefile, uint8_t* data1[],
                       uint8_t* data2[]) {
  make_fileL00(temp_File1, temp_File2, mergefile);
  write_headerL00(temp_File1, temp_File2, mergefile);
  write_frameL00(temp_File1, temp_File2, mergefile, data1, data2);
}
void merge_BinFile_L11(LegacyBinFileInfo_t* temp_File1,
                       LegacyBinFileInfo_t* temp_File2,
                       BinDataInfo_t* convertBinFileInf1,
                       BinDataInfo_t* convertBinFileInf2,
                       BinDataInfo_t* mergefile) {
  make_fileL11(temp_File1, temp_File2, mergefile);
  write_headerL11(temp_File1, temp_File2, convertBinFileInf1,
                  convertBinFileInf2, mergefile);
  write_frameL11(temp_File1, temp_File2, convertBinFileInf1, convertBinFileInf2,
                 mergefile);
}
void merge_BinFile_L10(LegacyBinFileInfo_t* temp_File1,
                       BinDataInfo_t* temp_File2,
                       BinDataInfo_t* convertBinFileInf1,
                       BinDataInfo_t* mergefile) {
  make_fileL10(temp_File1, temp_File2, mergefile);
  write_headerL10(temp_File1, temp_File2, convertBinFileInf1, mergefile);
  write_frameL10(temp_File1, temp_File2, convertBinFileInf1, mergefile);
}
void merge_BinFile_L01(BinDataInfo_t* temp_File1,
                       LegacyBinFileInfo_t* temp_File2,
                       BinDataInfo_t* convertBinFileInf1,
                       BinDataInfo_t* mergefile) {
  make_fileL01(temp_File1, temp_File2, mergefile);
  write_headerL01(temp_File1, temp_File2, convertBinFileInf1, mergefile);
  write_frameL01(temp_File1, temp_File2, convertBinFileInf1, mergefile);
}