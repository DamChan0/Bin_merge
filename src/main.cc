#include "main.h"

#include <string.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
using namespace std;
uint8_t* framedata1[100];
uint8_t* framedata2[100];
static BinDataInfo_t bin_File1;  // 1번파일 정보
static BinDataInfo_t bin_File2;  // 2번파일 정보
// static BinDataInfo_t temp_File1;  // 1번파일 읽은 파일 복사
// static BinDataInfo_t temp_File2;  // 2번파일 읽은 파일 복사
static LegacyBinFileInfo_t
  leg_bin_File2;  // 1번파일이 레거시 일 때 읽은 파일 복사
static LegacyBinFileInfo_t
  leg_bin_File1;  // 2번파일이 레거시 일 때 읽은 파일 복사

BinDataInfo_t merged_File;                // 실제 쓰여질 파일 정보
static BinDataInfo_t convertBinFileInf1;  // 실제 쓰여질 파일 정보
static BinDataInfo_t convertBinFileInf2;  // 실제 쓰여질 파일 정보

// 임시 파일 합치기

bool legFlag1 = false;
bool legFlag2 = false;

int openfile(char* filePath1, char* filePath2) {
  if (filePath1 == NULL || filePath2 == NULL) {
    return 1;
  } else if (filePath1 != NULL && filePath2 != NULL) {
    cout << "filePath1 :" << filePath1 << "\n";
    cout << "filePath2 :" << filePath2 << "\n \n";
  }
  return 0;
}

int main(int argc, char* argv[]) {
  char senNumStr[1024] = {'\0'};
  if (argc < 3) {
    cout << "Please provide two file names."
         << "\n";
    return 1;
  }
  char* filePath1 = argv[1];
  char* filePath2 = argv[2];

  if (openfile(filePath1, filePath2)) {
    return 1;
  }

  ifstream inputFile1(filePath1);
  ifstream inputFile2(filePath2);

  if (!inputFile1.is_open()) {
    std::string errorMessage = "Failed to open file: " + std::string(filePath1);
    std::cerr << "Error: " << errorMessage << "\n"
              << " at " << __FILE__ << ":" << __LINE__ << ": "
              << strerror(errno) << "\n\n";
    exit(EXIT_FAILURE);
  }

  else {
    clog << "Success to open file!"
         << "\n";
  }

  if (!inputFile2.is_open()) {
    std::string errorMessage = "Failed to open file: " + std::string(filePath2);
    std::cerr << "Error: " << errorMessage << "\n"
              << " at " << __FILE__ << ":" << __LINE__ << ": "
              << strerror(errno) << "\n\n";
    exit(EXIT_FAILURE);
  }

  else {
    clog << "Success to open file!"
         << "\n";
  }
  char binVersion1[1024];
  char binVersion2[1024];

  inputFile1.read(binVersion1, sizeof(binVersion1));
  inputFile2.read(binVersion2, sizeof(binVersion2));

  BinDataInfo_t temp;
  // check version and make legacy branch
  // if (((BinHeadInfo_t*)binVersion1)->majorVer == 1) {
  // cout << "Bin1 not legacy file!!"
  //      << "\n";
  inputFile1.close();
  // file pointer 적용 맞는 structure에 적용

  strcpy(bin_File1.filePath, filePath1);
  strcpy(bin_File2.filePath, filePath2);
  bin_File1.filePtr = fopen(bin_File1.filePath, "r");
  bin_File2.filePtr = fopen(bin_File2.filePath, "r");
  make_fileL00(&bin_File1, &bin_File2, &temp);
  read_BinHeader(&bin_File1);
  read_BinHeader(&bin_File2);
  write_headerL00(&bin_File1, &bin_File2, &temp);
  read_FrameData(&bin_File1, &bin_File2, &temp);

  // } else if (((LegacyBinHead_t*)binVersion1)->ver > BIN_LEG_VER_1 &&
  //            ((LegacyBinHead_t*)binVersion1)->ver < BIN_LEG_VER_MAX) {
  //   cout << "Bin Leagcy file1!!"
  //        << "\n";

  //   legFlag1 = true;
  //   leg_bin_File1.filePath = filePath1;
  //   leg_bin_File1.file = &inputFile1;
  //   leg_bin_File1.ver = (eLegacyBinVer)((LegacyBinHead_t*)binVersion1)->ver;
  //   leg_bin_File1.defInfo = getBinLegacyElementInfo(leg_bin_File1.ver);
  //   cout << "lidar num: " << leg_bin_File1.defInfo->lidarCnt << endl;
  //   inputFile1.seekg(0, ios::end);
  //   leg_bin_File1.fileSz = inputFile1.tellg();

  //   inputFile1.seekg(sizeof(LegacyBinHead_t), ios::beg);
  //   readLegacyBinHeader(&leg_bin_File1);
  //   readLegacyBinLidarCal(*leg_bin_File1.file,
  //   leg_bin_File1.defInfo->lidarCnt); indexingLegacyBin(&leg_bin_File1);
  //   setSensorIdFromString(senNumStr, &leg_bin_File1);

  //   // // set new bin header
  //   // inputFile.seekg(0, ios::beg);
  //   // binHeadInitFromLegacyBin(&convertBinFileInf, &leg_bin_File1);
  //   // binSenserHeadInitFromLegacyBin(&convertBinFileInf, &leg_bin_File1);
  //   // binSetCalInitFromLegacyBin(&convertBinFileInf, &leg_bin_File1);
  //   // binWriteBodyFromLegacyBin(&convertBinFileInf, &leg_bin_File1);

  //   // convertBinFileInf.ofile.close();
  //   // leg_bin_File1.file->close();

  //   // strcpy(binFileInf.fileName, convertBinFileInf.fileName);
  //   // newBinFileInf.lidarNum = desiredNum;
  //   // volumeUpBinFile(&binFileInf, &newBinFileInf);
  // }

  // if (((BinHeadInfo_t*)binVersion2)->majorVer == 1) {
  //   cout << "Bin2 not legacy file!!"
  //        << "\n";
  //   inputFile2.close();
  //   // file pointer 적용 맞는 structure에 적용

  //   strcpy(bin_File2.filePath, filePath2);
  //   bin_File2.filePtr = fopen(bin_File2.filePath, "r");

  //   read_BinHeader(&bin_File2);
  //   read_FrameData(&bin_File2, framedata2);

  // } else if (((LegacyBinHead_t*)binVersion2)->ver > BIN_LEG_VER_1 &&
  //            ((LegacyBinHead_t*)binVersion2)->ver < BIN_LEG_VER_MAX) {
  //   cout << "Bin2 Leagcy file!!"
  //        << "\n";

  //   legFlag2 = true;
  //   leg_bin_File2.filePath = filePath1;
  //   leg_bin_File2.file = &inputFile2;
  //   leg_bin_File2.ver = (eLegacyBinVer)((LegacyBinHead_t*)binVersion2)->ver;
  //   leg_bin_File2.defInfo = getBinLegacyElementInfo(leg_bin_File2.ver);
  //   cout << "lidar num: " << leg_bin_File2.defInfo->lidarCnt << endl;
  //   inputFile2.seekg(0, ios::end);
  //   leg_bin_File2.fileSz = inputFile2.tellg();
  // }

  // if (!legFlag1 && !legFlag2) {
  //   std::cout << "\n";
  //   std::cout << "merge version is not Leagcay1";

  //   merge_BinFile_L00(&bin_File1, &bin_File2, &merged_File, framedata1,
  //                     framedata2);
  // } else if (legFlag1 && legFlag2) {
  //   merge_BinFile_L11(&leg_bin_File1, &leg_bin_File2, &convertBinFileInf1,
  //                     &convertBinFileInf1, &merged_File);

  // } else if (legFlag1 && !legFlag2) {
  //   merge_BinFile_L10(&leg_bin_File1, &bin_File2, &convertBinFileInf1,
  //                     &merged_File);

  // } else if (!legFlag1 && legFlag2) {
  //   merge_BinFile_L01(&bin_File1, &leg_bin_File2, &convertBinFileInf1,
  //                     &merged_File);
  // }

  // mergeBinFile(&filePath1, &filePath2);
  // make new file and open file

  // read file  Header and copy header and write headre

  // read file frame data and copt header and write frame
}