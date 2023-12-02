#include "main.h"

#include <string.h>
#include <unistd.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
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

BinFileInfo_t f1;
BinFileInfo_t f2;
BinFileInfo_t target;

// Function to print a warning message in red color
void printWarning(const std::string& message) {
  std::cout << std::endl
            << ANSI_COLOR_PURPLE << "Warning!!!: " << message
            << ANSI_COLOR_RESET << std::endl
            << std::endl;
}

void printTime(uint32_t sec, uint32_t min) {
  std::cout << ANSI_COLOR_PURPLE ANSI_COLOR_WHITE BOLD << "PLAYTIME: " << min
            << "분" << sec << "초" << ANSI_COLOR_RESET << std::endl
            << std::endl;
}

void printError(const std::string& message) {
  std::cout << std::endl
            << ANSI_COLOR_RED << "ERROR!!!: " << message << ANSI_COLOR_RESET
            << std::endl
            << std::endl;
  std::exit(1);  // 에러 코드 1로 종료
}

void printCorrect(const std::string& message) {
  std::cout << std::endl
            << ANSI_COLOR_GREEN << "Correct!: " << message << ANSI_COLOR_RESET
            << std::endl
            << std::endl;
}

void printDebug(const std::string& message, const std::string& flag) {
  std::cout << ANSI_COLOR_GREEN ANSI_COLOR_WHITE BOLD << flag << message
            << ANSI_COLOR_RESET << std::endl
            << std::endl;
}

int mergeBinFrame_time(BinFileInfo_t& src1, BinFileInfo_t& src2,
                       BinFileInfo_t& dst);

bool forceOption = 0;

void set_and_print_outputfile(BinFileInfo_t& output, int frameNum) {
  std::unique_ptr<char[]> frameHeader(new char[sizeof(BinFrameHead2d0_t)]);

  output.frameNum = frameNum;
  std::string fileName = output.fileName;
  std::filesystem::path currentPath = std::filesystem::current_path();

  output.file.open(output.fileName, ios::in | ios::out);

  std::filesystem::path absolutePath = currentPath / fileName;
  std::cout << std::endl;
  output.fileName = absolutePath;
  output.size = std::filesystem::file_size(output.fileName);
  printDebug(output.fileName, "File absolutePath: ");
  std::string sizeString = std::to_string(output.size / (1024 * 1024));
  std::string frameeString = std::to_string(output.frameNum - 1);
  printDebug(sizeString, "File size(MB): ");
  printDebug(frameeString, "Total FrameNum: ");
}

int checkOpt(string& file1, string& file2, int argc, char* argv[]) {
  if (argc == 1) {
    cout << "Error: Argument Input Error" << endl;
    return 1;
  }
  int opt;
  while ((opt = getopt(argc, argv, OPT_FMT)) != -1) {
    switch (opt) {
      case 'f':
        printDebug("merge file forced", "MergeOption: ");
        forceOption = true;
        break;
      case 'd':
        printDebug("merge file by timestamp", "MergeOption: ");
        forceOption = false;
        break;
      case '1':
        file1 = optarg;
        // cout << file1 << endl;
        break;
      case '2':
        file2 = optarg;
        // cout << file2 << endl;
        break;
      default:
        printf("Unknown option: -%c\n", optopt);
        return 1;
    }
  }

  if (file1.length() == 0 || file2.length() == 2) {
    cout << "Error: Argument Input Error" << endl;
    return 1;
  }

  return 0;
}

int loadBinHeader(BinFileInfo_t& inf) {
  inf.file.open(inf.fileName, ios::in);
  if (!inf.file.is_open()) {
    cout << "Cannot open " << inf.fileName << endl;
    return 1;
  }

  filesystem::path fPath = inf.fileName;

  // check size
  inf.size = std::filesystem::file_size(fPath);

  // load header
  inf.file.read((char*)&inf.binHead, sizeof(inf.binHead));

  // validation check
  string marker(HD_MARKER);
  string infMarker(inf.binHead.marker);
  if (marker != infMarker) {
    cout << "Invalid bin file" << inf.fileName << endl;
    return 1;
  }
  cout << inf.fileName << endl;

  cout << inf.binHead.str << endl;

  // load sensors info
  inf.file.read((char*)inf.sensors,
                sizeof(*inf.sensors) * inf.binHead.sensorCnt);

  for (size_t i = 0; i < inf.binHead.sensorCnt; i++) {
    cout << inf.sensors[i].str << endl;
  }

  // load cal info
  inf.file.read((char*)inf.cal, sizeof(*inf.cal) * inf.binHead.calCnt);
  for (size_t i = 0; i < inf.binHead.calCnt; i++) {
    cout << inf.cal[i].str << endl;
  }

  inf.lidarNum = inf.binHead.calCnt;

  // calc frame num
  inf.frameNum = (inf.size - inf.binHead.headerSz) / inf.binHead.singleFrameSz;

  cout << "total frame no: " << inf.frameNum << endl;

  return 0;
}

void checkBinVersion(BinFileInfo_t& file) {
  if (file.binHead.majorVer == BIN_MAJOR_VER_2) {
    file.majorVer = BIN_MAJOR_VER_2;
  } else {
    file.majorVer = BIN_MAJOR_VER_NONE;
  }
}

size_t getFrameHeadSzUsingVersion(int ver1, int ver2) {
  if (ver1 != BIN_MAJOR_VER_2 && ver2 != BIN_MAJOR_VER_2) {
    printDebug("BinVer1 + BinVer1", "BinVersion: ");
    return sizeof(BinFrameHead_t);
  } else if (ver1 != BIN_MAJOR_VER_2 && ver2 == BIN_MAJOR_VER_2) {
    if (f2.binHead.minorVer == 1) {
      printDebug("BinVer1 + BinVer2", "BinVersion: ");
      return sizeof(BinFrameHead_t) + f2.lidarNum * sizeof(TimeStamp_t);
    } else {
      printDebug("BinVer1 + BinVer2", "BinVersion: ");
      return sizeof(BinFrameHead_t);
    }
    printDebug("BinVer1 + BinVer2", "BinVersion: ");
  } else if (ver1 == BIN_MAJOR_VER_2 && ver2 != BIN_MAJOR_VER_2) {
    if (f1.binHead.minorVer == 1) {
      printDebug("BinVer2 + BinVer1", "BinVersion: ");
      return sizeof(BinFrameHead_t) + f1.lidarNum * sizeof(TimeStamp_t);
    }
    printDebug("BinVer2 + BinVer1", "BinVersion: ");
    return sizeof(BinFrameHead_t);
  } else if (ver1 == BIN_MAJOR_VER_2 && ver2 == BIN_MAJOR_VER_2) {
    if (f1.binHead.minorVer == 1 && f2.binHead.minorVer == 1) {
      printDebug("BinVer2.1 + BinVer2.1", "BinVersion: ");
      return sizeof(BinFrameHead2d0_t);
    } else if ((f1.binHead.minorVer == 1 && f2.binHead.minorVer != 1)) {
      printDebug("BinVer2.1 + BinVer2.0", "BinVersion: ");
      return sizeof(BinFrameHead2d0_t) + f1.lidarNum * sizeof(TimeStamp_t);
    } else if ((f1.binHead.minorVer != 1 && f2.binHead.minorVer == 1)) {
      printDebug("BinVer2.0 + BinVer2.1", "BinVersion: ");
      return sizeof(BinFrameHead2d0_t) + f2.lidarNum * sizeof(TimeStamp_t);
    } else {
      printDebug("BinVer2 + BinVer2", "BinVersion: ");
      return sizeof(BinFrameHead2d0_t);
    }
  }

  return 0;
}

int mergeBinHeader(BinFileInfo_t& src1, BinFileInfo_t& src2,
                   BinFileInfo_t& dst) {
  // configure file name
  filesystem::path fname1 = src1.fileName;

  filesystem::path fname2 = src2.fileName;

  string f1NameExceptExt = fname1.stem().string();
  string f2NameExceptExt = fname2.stem().string();
  if (forceOption) {
    dst.fileName = f1NameExceptExt + "_" + f2NameExceptExt + "_Force" +
                   fname1.extension().string();
  } else {
    dst.fileName = f1NameExceptExt + "_" + f2NameExceptExt + "_Match_Time" +
                   fname1.extension().string();
  }

  // open file
  dst.file.open(dst.fileName, ios::out);
  if (!dst.file.is_open()) {
    cout << "Cannot Open New file: " << dst.fileName;
    return 1;
  }

  // fill Header
  strcpy(dst.binHead.marker, HD_MARKER);
  if (src1.binHead.majorVer == BIN_MAJOR_VER_2 &&
      src2.binHead.majorVer == BIN_MAJOR_VER_2) {
    if (src1.binHead.minorVer == BIN_MINOR_VER_1 &&
        src2.binHead.minorVer == BIN_MINOR_VER_1) {
      dst.binHead.majorVer = BIN_MAJOR_VER_2;
      dst.binHead.minorVer = BIN_MAJOR_VER_1;
    } else if ((src1.binHead.minorVer != BIN_MINOR_VER_1 &&
                src2.binHead.minorVer == BIN_MINOR_VER_1) ||
               (src1.binHead.minorVer == BIN_MINOR_VER_1 &&
                src2.binHead.minorVer != BIN_MINOR_VER_1)) {
      dst.binHead.majorVer = BIN_MAJOR_VER_2;
      dst.binHead.minorVer = BIN_MAJOR_VER_NONE;
    } else {
      dst.binHead.majorVer = src1.binHead.majorVer;
      dst.binHead.minorVer = src1.binHead.minorVer;
    }
  } else if ((src1.majorVer == BIN_MAJOR_VER_2 &&
              src2.majorVer != BIN_MAJOR_VER_2) ||
             (src1.majorVer != BIN_MAJOR_VER_2 &&
              src2.majorVer == BIN_MAJOR_VER_2)) {
    dst.binHead.majorVer = BIN_MAJOR_VER_2;
    dst.binHead.minorVer = BIN_MAJOR_VER_NONE;
  } else {
    dst.binHead.majorVer = src1.binHead.majorVer;
    dst.binHead.minorVer = src1.binHead.minorVer;
  }
  // copy only lidar sensor
  dst.binHead.sensorCnt = src1.binHead.calCnt + src2.binHead.calCnt;
  dst.binHead.calCnt = src1.binHead.calCnt + src2.binHead.calCnt;
  dst.lidarNum = dst.binHead.calCnt;

  dst.binHead.headerSz = sizeof(dst.binHead) + sizeof(*dst.cal) * dst.lidarNum +
                         sizeof(*dst.sensors) * dst.lidarNum;

  size_t frameHeaderSz =
    getFrameHeadSzUsingVersion(src1.binHead.majorVer, src2.binHead.majorVer);
  dst.binHead.singleFrameSz =
    src1.binHead.singleFrameSz + src2.binHead.singleFrameSz - frameHeaderSz;

  dst.frameNum = src1.frameNum < src2.frameNum ? src1.frameNum : src2.frameNum;

  sprintf(dst.binHead.str,
          "Ver:%02d.%02d, hd_sz: %d, senCnt:%d, calCnt:%d,singleFrmSz:%d",
          dst.binHead.majorVer, dst.binHead.minorVer, dst.binHead.headerSz,
          dst.binHead.sensorCnt, dst.binHead.calCnt, dst.binHead.singleFrameSz);

  memcpy(dst.sensors, src1.sensors, sizeof(*dst.sensors) * src1.lidarNum);
  memcpy(&dst.sensors[src1.lidarNum], src2.sensors,
         sizeof(*dst.sensors) * src2.lidarNum);

  for (size_t i = 0; i < dst.lidarNum; i++) {
    memset(dst.sensors[i].subId, 0, sizeof(dst.sensors[i].subId));
    sprintf(dst.sensors[i].subId, "#%02ld", i);
  }

  memcpy(dst.cal, src1.cal, sizeof(*dst.cal) * src1.lidarNum);
  memcpy(&dst.cal[src1.lidarNum], src2.cal, sizeof(*dst.cal) * src2.lidarNum);

  dst.file.write((char*)&dst.binHead, sizeof(dst.binHead));
  dst.file.write((char*)dst.sensors, sizeof(*dst.sensors) * dst.lidarNum);
  dst.file.write((char*)dst.cal, sizeof(*dst.cal) * dst.lidarNum);
  return 0;
}

// 두파일중에 Binver2.0이 없는 경우
int mergeBinFrame11(BinFileInfo_t& src1, BinFileInfo_t& src2,
                    BinFileInfo_t& dst) {
  BinFrameHead_t frmHead;
  std::unique_ptr<char[]> dataBuffer1(new char[src1.binHead.singleFrameSz]);
  std::unique_ptr<char[]> dataBuffer2(new char[src2.binHead.singleFrameSz]);
  std::unique_ptr<char[]> frameHeader_Buffer1(
    new char[sizeof(BinFrameHead2d0_t)]);
  std::unique_ptr<char[]> frameHeader_Buffer2(
    new char[sizeof(BinFrameHead2d0_t)]);
  check_file_length(frameHeader_Buffer1, frameHeader_Buffer2, src1, src2);

  for (size_t i = 0; i < dst.frameNum; i++) {
    memset(&frmHead, 0, sizeof(frmHead));
    strcpy(frmHead.frameMarker, FRAME_MARKER);
    frmHead.frameNum = i;
    sprintf(frmHead.str, "#%d", frmHead.frameNum);

    dst.file.write((char*)&frmHead, sizeof(frmHead));

    src1.file.seekg(src1.binHead.headerSz + src1.binHead.singleFrameSz * i);
    src2.file.seekg(src2.binHead.headerSz + src2.binHead.singleFrameSz * i);

    src1.file.read(dataBuffer1.get(), src1.binHead.singleFrameSz);
    src2.file.read(dataBuffer2.get(), src2.binHead.singleFrameSz);

    dst.file.write(&dataBuffer1[sizeof(frmHead)],
                   src1.binHead.singleFrameSz - sizeof(frmHead));
    dst.file.write(&dataBuffer2[sizeof(frmHead)],
                   src2.binHead.singleFrameSz - sizeof(frmHead));
  }
  dst.file.close();

  set_and_print_outputfile(dst, dst.frameNum);

  return 0;
}

void is_firstVer2(std::unique_ptr<char[]>& dataBuffer1,
                  std::unique_ptr<char[]>& dataBuffer2,
                  std::unique_ptr<char[]>& frameHeader_Buffer1,
                  BinFileInfo_t& src1, BinFileInfo_t& src2, BinFileInfo_t& dst,
                  int frameNum) {
  src1.file.seekg(src1.binHead.headerSz +
                  src1.binHead.singleFrameSz * frameNum);
  src1.file.read(frameHeader_Buffer1.get(), sizeof(BinFrameHead2d0_t));

  BinFrameHead2d0_t* file1_frmHead2d0 =
    reinterpret_cast<BinFrameHead2d0_t*>(frameHeader_Buffer1.get());

  dst.file.write(reinterpret_cast<char*>(file1_frmHead2d0),
                 sizeof(BinFrameHead2d0_t));

  src1.file.seekg(src1.binHead.headerSz +
                  src1.binHead.singleFrameSz * frameNum);
  src2.file.seekg(src2.binHead.headerSz +
                  src2.binHead.singleFrameSz * frameNum);

  src1.file.read(dataBuffer1.get(), src1.binHead.singleFrameSz);
  src2.file.read(dataBuffer2.get(), src2.binHead.singleFrameSz);

  dst.file.write(&dataBuffer1[sizeof(BinFrameHead2d0_t)],
                 src1.binHead.singleFrameSz - sizeof(BinFrameHead2d0_t));
  dst.file.write(&dataBuffer2[sizeof(BinFrameHead_t)],
                 src2.binHead.singleFrameSz - sizeof(BinFrameHead_t));
}

void is_secondVer2(std::unique_ptr<char[]>& dataBuffer1,
                   std::unique_ptr<char[]>& dataBuffer2,
                   std::unique_ptr<char[]>& frameHeader_Buffer2,
                   BinFileInfo_t& src1, BinFileInfo_t& src2, BinFileInfo_t& dst,
                   int frameNum) {
  src2.file.seekg(src2.binHead.headerSz +
                  src2.binHead.singleFrameSz * frameNum);
  src2.file.read(frameHeader_Buffer2.get(), sizeof(BinFrameHead2d0_t));

  BinFrameHead2d0_t* file2_frmHead2d0 =
    reinterpret_cast<BinFrameHead2d0_t*>(frameHeader_Buffer2.get());

  dst.file.write(reinterpret_cast<char*>(file2_frmHead2d0),
                 sizeof(BinFrameHead2d0_t));

  src1.file.seekg(src1.binHead.headerSz +
                  src1.binHead.singleFrameSz * frameNum);
  src2.file.seekg(src2.binHead.headerSz +
                  src2.binHead.singleFrameSz * frameNum);

  src1.file.read(dataBuffer1.get(), src1.binHead.singleFrameSz);
  src2.file.read(dataBuffer2.get(), src2.binHead.singleFrameSz);

  dst.file.write(&dataBuffer1[sizeof(BinFrameHead_t)],
                 src1.binHead.singleFrameSz - sizeof(BinFrameHead_t));
  dst.file.write(&dataBuffer2[sizeof(BinFrameHead2d0_t)],
                 src2.binHead.singleFrameSz - sizeof(BinFrameHead2d0_t));
}

void is_firstVer2d1(std::unique_ptr<char[]>& dataBuffer1,
                    std::unique_ptr<char[]>& dataBuffer2,
                    std::unique_ptr<char[]>& timestamp_buffer,
                    std::unique_ptr<char[]>& frameHeader_Buffer1,
                    BinFileInfo_t& src1, BinFileInfo_t& src2,
                    BinFileInfo_t& dst, int frameNum) {
  src1.file.seekg(src1.binHead.headerSz +
                  src1.binHead.singleFrameSz * frameNum);
  src1.file.read(frameHeader_Buffer1.get(), sizeof(BinFrameHead2d0_t));
  for (uint32_t i = 0; i < src1.lidarNum; i++) {
    src1.file.read(timestamp_buffer.get() + sizeof(TimeStamp_t) * i,
                   sizeof(TimeStamp_t));
  }

  BinFrameHead2d0_t* file1_frmHead2d0 =
    reinterpret_cast<BinFrameHead2d0_t*>(frameHeader_Buffer1.get());

  std::streampos dstp;
  dst.file.write(reinterpret_cast<char*>(file1_frmHead2d0),
                 sizeof(BinFrameHead2d0_t));

  // Ver 2.1 framedata position
  src1.file.seekg(
    src1.binHead.headerSz + src1.binHead.singleFrameSz * frameNum +
    sizeof(BinFrameHead2d0_t) + sizeof(TimeStamp_t) * src1.lidarNum);

  src1.file.read(dataBuffer1.get(), src1.binHead.singleFrameSz -
                                      sizeof(BinFrameHead2d0_t) +
                                      sizeof(TimeStamp_t) * src1.lidarNum);
  // Ver 1.2 framedata position
  src2.file.seekg(src2.binHead.headerSz +
                  src2.binHead.singleFrameSz * frameNum);

  src2.file.read(dataBuffer2.get(), src2.binHead.singleFrameSz);

  dst.file.write(dataBuffer1.get(), src1.binHead.singleFrameSz -
                                      sizeof(BinFrameHead2d0_t) -
                                      sizeof(TimeStamp_t) * src1.lidarNum);

  dst.file.write(&dataBuffer2[sizeof(BinFrameHead_t)],
                 src2.binHead.singleFrameSz - sizeof(BinFrameHead_t));
}

void is_secondVer2d1(std::unique_ptr<char[]>& dataBuffer1,
                     std::unique_ptr<char[]>& dataBuffer2,
                     std::unique_ptr<char[]>& timestamp_buffer,
                     std::unique_ptr<char[]>& frameHeader_Buffer2,
                     BinFileInfo_t& src1, BinFileInfo_t& src2,
                     BinFileInfo_t& dst, int frameNum) {
  src2.file.seekg(src2.binHead.headerSz +
                  src2.binHead.singleFrameSz * frameNum);
  src2.file.read(frameHeader_Buffer2.get(), sizeof(BinFrameHead2d0_t));
  for (uint32_t i = 0; i < src2.lidarNum; i++) {
    src2.file.read(timestamp_buffer.get() + sizeof(TimeStamp_t) * i,
                   sizeof(TimeStamp_t));
  }

  BinFrameHead2d0_t* file2_frmHead2d0 =
    reinterpret_cast<BinFrameHead2d0_t*>(frameHeader_Buffer2.get());

  dst.file.write(reinterpret_cast<char*>(file2_frmHead2d0),
                 sizeof(BinFrameHead2d0_t));
  // Ver 2.1 framedata position
  src2.file.seekg(
    src2.binHead.headerSz + src2.binHead.singleFrameSz * frameNum +
    sizeof(BinFrameHead2d0_t) + sizeof(TimeStamp_t) * src2.lidarNum);
  src2.file.read(dataBuffer2.get(), src2.binHead.singleFrameSz -
                                      sizeof(BinFrameHead2d0_t) +
                                      sizeof(TimeStamp_t) * src2.lidarNum);
  // Ver 1.2 framedata position
  src1.file.seekg(src1.binHead.headerSz +
                  src1.binHead.singleFrameSz * frameNum);
  src1.file.read(dataBuffer1.get(), src1.binHead.singleFrameSz);

  dst.file.write(&dataBuffer1[sizeof(BinFrameHead_t)],
                 src1.binHead.singleFrameSz - sizeof(BinFrameHead_t));

  dst.file.write(dataBuffer2.get(), src2.binHead.singleFrameSz -
                                      sizeof(BinFrameHead2d0_t) -
                                      sizeof(TimeStamp_t) * src2.lidarNum);
}

int mergeBinFrame1_2d1(BinFileInfo_t& src1, BinFileInfo_t& src2,
                       BinFileInfo_t& dst) {
  std::unique_ptr<char[]> dataBuffer1(new char[src1.binHead.singleFrameSz]);
  std::unique_ptr<char[]> dataBuffer2(new char[src2.binHead.singleFrameSz]);

  std::unique_ptr<char[]> frameHeader_Buffer1(
    new char[sizeof(BinFrameHead2d0_t)]);
  std::unique_ptr<char[]> frameHeader_Buffer2(
    new char[sizeof(BinFrameHead2d0_t)]);

  std::unique_ptr<char[]> timeStamp_Buffer1(
    new char[sizeof(TimeStamp_t) * src1.lidarNum]);
  std::unique_ptr<char[]> timeStamp_Buffer2(
    new char[sizeof(TimeStamp_t) * src2.lidarNum]);

  for (size_t i = 0; i < dst.frameNum; i++) {
    // 첫번째 파일이 Binver2.1
    if (src1.binHead.majorVer == BIN_MAJOR_VER_2 &&
        src2.binHead.majorVer != BIN_MAJOR_VER_2) {
      is_firstVer2d1(dataBuffer1, dataBuffer2, timeStamp_Buffer1,
                     frameHeader_Buffer1, src1, src2, dst, i);
    }
    // 두번째 파일이 Binver2.1
    else if (src1.binHead.majorVer != BIN_MAJOR_VER_2 &&
             src2.binHead.majorVer == BIN_MAJOR_VER_2) {
      is_secondVer2d1(dataBuffer1, dataBuffer2, timeStamp_Buffer2,
                      frameHeader_Buffer2, src1, src2, dst, i);
    }

    dst.file.flush();
    if (i % 20 == 0) {
      if (i % 400 == 0) {
        std::cout << std::endl;
      }
      std::cout << '#';
      flush(cout);
    }
  }
  dst.file.close();
  set_and_print_outputfile(dst, dst.frameNum);
  return 0;
}

// 두파일중에 하나라도 Binver2.0이 있는 경우
int mergeBinFrame12(BinFileInfo_t& src1, BinFileInfo_t& src2,
                    BinFileInfo_t& dst) {
  std::unique_ptr<char[]> dataBuffer1(new char[src1.binHead.singleFrameSz]);
  std::unique_ptr<char[]> dataBuffer2(new char[src2.binHead.singleFrameSz]);
  std::unique_ptr<char[]> frameHeader_Buffer1(
    new char[sizeof(BinFrameHead2d0_t)]);
  std::unique_ptr<char[]> frameHeader_Buffer2(
    new char[sizeof(BinFrameHead2d0_t)]);
  check_file_length(frameHeader_Buffer1, frameHeader_Buffer2, src1, src2);

  for (size_t i = 0; i < dst.frameNum; i++) {
    // 첫번째 파일이 Binver2.0
    if (src1.binHead.majorVer == BIN_MAJOR_VER_2 &&
        src2.binHead.majorVer != BIN_MAJOR_VER_2) {
      is_firstVer2(dataBuffer1, dataBuffer2, frameHeader_Buffer1, src1, src2,
                   dst, i);
    }
    // 두번째 파일이 Binver2.0
    else if (src1.binHead.majorVer != BIN_MAJOR_VER_2 &&
             src2.binHead.majorVer == BIN_MAJOR_VER_2) {
      is_secondVer2(dataBuffer1, dataBuffer2, frameHeader_Buffer2, src1, src2,
                    dst, i);
    }

    dst.file.flush();
    if (i % 20 == 0) {
      if (i % 400 == 0) {
        std::cout << std::endl;
      }
      std::cout << '#';
      flush(cout);
    }
  }
  dst.file.close();
  set_and_print_outputfile(dst, dst.frameNum);
  return 0;
}

// 하나의 파일이라도 Binver2.0이 아닌 경우
int mergeBinFrame(BinFileInfo_t& src1, BinFileInfo_t& src2,
                  BinFileInfo_t& dst) {
  if (src1.binHead.majorVer == 1 && src2.binHead.majorVer == 1) {
    mergeBinFrame11(src1, src2, dst);
  } else {
    if ((src1.binHead.majorVer == 2 && src1.binHead.minorVer == 1) ||
        (src2.binHead.majorVer == 2 && src2.binHead.minorVer == 1)) {
      mergeBinFrame1_2d1(src1, src2, dst);
    } else {
      mergeBinFrame12(src1, src2, dst);
    }
  }
  return 0;
}

uint64_t time_diff(BinFrameHead2d0_t* file1_frmHead2d0,
                   BinFrameHead2d0_t* file2_frmHead2d0) {
  return std::abs(static_cast<int64_t>(file1_frmHead2d0->timelog -
                                       file2_frmHead2d0->timelog));
}

uint64_t time_diff2d1(TimeStamp_t* timeS1, TimeStamp_t* timeS2) {
  // 마지막 time_Stamp로 seekg 한 후
  uint64_t file1 = timeS1->timelog;
  uint64_t file2 = timeS2->timelog;
  return std::abs(static_cast<int64_t>(file1 - file2));
}

int merge_possibility(int start1, int start2, int end1, int end2) {
  int possibility;

  if (end1 < start2 || start1 > end2) {
    possibility = 1;
  } else if (std::abs(start1 - start2) / (1000 * 60) >= 5) {
    possibility = 2;
  } else {
    possibility = 3;
  }
  return possibility;
}

// 파일 길이 파악하여 merge 가능 여부 판단하여 로그 출력
void check_file_length(std::unique_ptr<char[]>& frameHeader_Buffer1,
                       std::unique_ptr<char[]>& frameHeader_Buffer2,
                       BinFileInfo_t& src1, BinFileInfo_t& src2) {
  int do_merge;

  src1.file.seekg(src1.binHead.headerSz + src1.binHead.singleFrameSz * 0);
  src2.file.seekg(src2.binHead.headerSz + src2.binHead.singleFrameSz * 0);

  src1.file.read(frameHeader_Buffer1.get(), sizeof(BinFrameHead2d0_t));
  src2.file.read(frameHeader_Buffer2.get(), sizeof(BinFrameHead2d0_t));

  BinFrameHead2d0_t* file1_frmHead2d0 =
    reinterpret_cast<BinFrameHead2d0_t*>(frameHeader_Buffer1.get());

  BinFrameHead2d0_t* file2_frmHead2d0 =
    reinterpret_cast<BinFrameHead2d0_t*>(frameHeader_Buffer2.get());
  [[maybe_unused]] uint64_t start_time1 = file1_frmHead2d0->timelog;
  [[maybe_unused]] uint64_t start_time2 = file2_frmHead2d0->timelog;

  src1.file.seekg(src1.binHead.headerSz +
                  src1.binHead.singleFrameSz * (src1.frameNum - 1));
  src2.file.seekg(src2.binHead.headerSz +
                  src2.binHead.singleFrameSz * (src2.frameNum - 1));

  src1.file.read(frameHeader_Buffer1.get(), sizeof(BinFrameHead2d0_t));
  src2.file.read(frameHeader_Buffer2.get(), sizeof(BinFrameHead2d0_t));

  file1_frmHead2d0 =
    reinterpret_cast<BinFrameHead2d0_t*>(frameHeader_Buffer1.get());

  file2_frmHead2d0 =
    reinterpret_cast<BinFrameHead2d0_t*>(frameHeader_Buffer2.get());
  uint64_t end_time1 = file1_frmHead2d0->timelog;
  [[maybe_unused]] uint64_t end_time2 = file2_frmHead2d0->timelog;

  do_merge = merge_possibility(start_time1, start_time2, end_time1, end_time2);
  if (do_merge == 3) {
    printCorrect("merger is valid. Merge is carried out");
  } else if (do_merge == 1) {
    printWarning("time diifernce is exist");
    printWarning("time diifernce is exist");
  } else if (do_merge == 2) {
    printError(
      "merger is Not valid, time differnce over 5min, Shut down the program");
  }

  uint64_t playtime1 = end_time1 - start_time1;
  uint64_t playtime2 = end_time2 - start_time2;

  uint64_t file_timediff =
    std::abs(static_cast<long>(start_time1 - start_time2));

  int time1 = static_cast<int>(playtime1) / 1000;
  int time2 = static_cast<int>(playtime2) / 1000;

  int mintime1 = time1 / 60;
  int mintime2 = time2 / 60;

  int sectime1 = time1 - mintime1 * 60;
  int sectime2 = time2 - mintime2 * 60;

  // 결과 출력
  std::cout << "파일 시차: " << file_timediff / 1000 << "초" << std::endl;

  std::cout << "1번 파일 길이: ";
  std::cout << mintime1 << "분" << sectime1 << "초" << std::endl;
  std::cout << "2번 파일 길이: ";
  std::cout << mintime2 << "분" << sectime2 << "초" << std::endl;
}

int mergeFrame2d1_timecomapre(BinFileInfo_t& src1, BinFileInfo_t& src2,
                              BinFileInfo_t& dst) {
  uint16_t min_frameNum = 0;
  std::unique_ptr<char[]> dataBuffer1(new char[src1.binHead.singleFrameSz]);
  std::unique_ptr<char[]> dataBuffer2(new char[src2.binHead.singleFrameSz]);

  std::unique_ptr<char[]> frameHeader_Buffer1(
    new char[sizeof(BinFrameHead2d0_t)]);
  std::unique_ptr<char[]> frameHeader_Buffer2(
    new char[sizeof(BinFrameHead2d0_t)]);

  std::unique_ptr<char[]> timeStamp_Buffer1(
    new char[sizeof(TimeStamp_t) * src1.lidarNum]);
  std::unique_ptr<char[]> timeStamp_Buffer2(
    new char[sizeof(TimeStamp_t) * src2.lidarNum]);

  // check_file_length(frameHeader_Buffer1, frameHeader_Buffer2, src1, src2);
  int newframeNum = 0;
  for (size_t i = 0; i < dst.frameNum; i++) {
    src1.file.seekg(src1.binHead.headerSz + src1.binHead.singleFrameSz * i);
    src2.file.seekg(src2.binHead.headerSz +
                    src2.binHead.singleFrameSz * min_frameNum);

    src1.file.read(frameHeader_Buffer1.get(), sizeof(BinFrameHead2d0_t));
    src2.file.read(frameHeader_Buffer2.get(), sizeof(BinFrameHead2d0_t));

    BinFrameHead2d0_t* file1_frmHead2d0 =
      reinterpret_cast<BinFrameHead2d0_t*>(frameHeader_Buffer1.get());

    BinFrameHead2d0_t* file2_frmHead2d0 =
      reinterpret_cast<BinFrameHead2d0_t*>(frameHeader_Buffer2.get());

    uint32_t min_time_diff = 100000000;

    UNUSED(min_frameNum);
    UNUSED(min_time_diff);
    uint64_t time = 0;

    for (uint32_t file2_frameNum = 0; file2_frameNum < dst.frameNum;
         file2_frameNum++) {
      src2.file.seekg(src2.binHead.headerSz +
                      src2.binHead.singleFrameSz * (file2_frameNum));
      src2.file.read(frameHeader_Buffer2.get(), sizeof(BinFrameHead2d0_t));
      file2_frmHead2d0 =
        reinterpret_cast<BinFrameHead2d0_t*>(frameHeader_Buffer2.get());
      time = time_diff(file1_frmHead2d0, file2_frmHead2d0);
      if (min_time_diff > time) {
        min_time_diff = time;
        min_frameNum = file2_frameNum;
      }
    }

    if (min_time_diff >= 200) {
      continue;
    } else if (time == 0) {
      min_frameNum = i;
      continue;
    }
    newframeNum++;
    file1_frmHead2d0->frameNum = newframeNum;

    // file2의 timeStamp 읽기
    if (min_frameNum == 0) {
      src2.file.seekg(src2.binHead.headerSz + src2.binHead.singleFrameSz * 0 +
                      sizeof(BinFrameHead2d0_t));
    } else {
      src2.file.seekg(src2.binHead.headerSz +
                      src2.binHead.singleFrameSz * (min_frameNum) +
                      sizeof(BinFrameHead2d0_t));
    }

    for (uint32_t j = 0; j < src2.lidarNum; j++) {
      src2.file.read(timeStamp_Buffer2.get() + j * sizeof(TimeStamp_t),
                     sizeof(TimeStamp_t));
      if (src2.file.fail()) {
        std::cerr << "Error reading from file: " << std::endl;
      }
    }
    // file1의 frameHeader 쓰기
    dst.file.write(reinterpret_cast<char*>(file1_frmHead2d0),
                   sizeof(BinFrameHead2d0_t));

    // file1,file2 timestamp 쓰기
    dst.file.write(timeStamp_Buffer1.get(), sizeof(TimeStamp_t));
    dst.file.write(timeStamp_Buffer2.get(), sizeof(TimeStamp_t));

    // file1, file2 data 찾기
    src1.file.seekg(src1.binHead.headerSz + src1.binHead.singleFrameSz * i);
    src2.file.seekg(src2.binHead.headerSz + src2.binHead.singleFrameSz * i);
    // file1, file2 data 읽기
    src1.file.read(dataBuffer1.get(), src1.binHead.singleFrameSz);
    src2.file.read(dataBuffer2.get(), src2.binHead.singleFrameSz);
    if (src2.file.fail()) {
      std::cerr << "Error reading from file: " << std::endl;
    }

    // file1, file2 data 쓰기
    dst.file.write(&dataBuffer1[sizeof(BinFrameHead2d0_t) +
                                sizeof(TimeStamp_t) * src1.lidarNum],
                   src1.binHead.singleFrameSz - sizeof(BinFrameHead2d0_t) -
                     sizeof(TimeStamp_t) * src1.lidarNum);

    dst.file.write(&dataBuffer2[sizeof(BinFrameHead2d0_t) +
                                sizeof(TimeStamp_t) * src2.lidarNum],
                   src2.binHead.singleFrameSz - sizeof(BinFrameHead2d0_t) -
                     sizeof(TimeStamp_t) * src2.lidarNum);

    if (i % 20 == 0) {
      if (i % 400 == 0) {
        std::cout << std::endl;
      }
      std::cout << '#';
      flush(cout);
    }
  }
  dst.file.close();
  return newframeNum;
}

int firsrt_ver2d1(BinFileInfo_t& src1, BinFileInfo_t& src2,
                  BinFileInfo_t& dst) {
  uint16_t min_frameNum = 0;
  std::unique_ptr<char[]> dataBuffer1(new char[src1.binHead.singleFrameSz]);
  std::unique_ptr<char[]> dataBuffer2(new char[src2.binHead.singleFrameSz]);

  std::unique_ptr<char[]> frameHeader_Buffer1(
    new char[sizeof(BinFrameHead2d0_t)]);
  std::unique_ptr<char[]> frameHeader_Buffer2(
    new char[sizeof(BinFrameHead2d0_t)]);

  std::unique_ptr<char[]> timeStamp_Buffer1(
    new char[sizeof(TimeStamp_t) * src1.lidarNum]);

  int newframeNum = 0;
  for (size_t i = 0; i < dst.frameNum; i++) {
    src1.file.seekg(src1.binHead.headerSz + src1.binHead.singleFrameSz * i);
    src2.file.seekg(src2.binHead.headerSz +
                    src2.binHead.singleFrameSz * min_frameNum);

    src1.file.read(frameHeader_Buffer1.get(), sizeof(BinFrameHead2d0_t));
    src2.file.read(frameHeader_Buffer2.get(), sizeof(BinFrameHead2d0_t));

    BinFrameHead2d0_t* file1_frmHead2d0 =
      reinterpret_cast<BinFrameHead2d0_t*>(frameHeader_Buffer1.get());

    BinFrameHead2d0_t* file2_frmHead2d0 =
      reinterpret_cast<BinFrameHead2d0_t*>(frameHeader_Buffer2.get());
    /*****************************************************************/

    uint32_t min_time_diff = 100000000;

    UNUSED(min_frameNum);
    UNUSED(min_time_diff);
    uint64_t time = 0;
    // time compare
    for (uint32_t file2_frameNum = 0; file2_frameNum < dst.frameNum;
         file2_frameNum++) {
      src2.file.seekg(src2.binHead.headerSz +
                      src2.binHead.singleFrameSz * (file2_frameNum));
      src2.file.read(frameHeader_Buffer2.get(), sizeof(BinFrameHead2d0_t));
      file2_frmHead2d0 =
        reinterpret_cast<BinFrameHead2d0_t*>(frameHeader_Buffer2.get());
      time = time_diff(file1_frmHead2d0, file2_frmHead2d0);
      if (min_time_diff > time) {
        min_time_diff = time;
        min_frameNum = file2_frameNum;
      }
    }

    if (min_time_diff >= 200) {
      continue;
    } else if (time == 0) {
      min_frameNum = i;
      continue;
    }
    newframeNum++;
    file1_frmHead2d0->frameNum = newframeNum;
    /*****************************************************************/
    // file1의 timeStamp 읽기
    if (min_frameNum == 0) {
      src1.file.seekg(src1.binHead.headerSz + src1.binHead.singleFrameSz * 0 +
                      sizeof(BinFrameHead2d0_t));
    } else {
      src1.file.seekg(src1.binHead.headerSz +
                      src1.binHead.singleFrameSz * (min_frameNum) +
                      sizeof(BinFrameHead2d0_t));
    }

    for (uint32_t j = 0; j < src1.lidarNum; j++) {
      src1.file.read(timeStamp_Buffer1.get() + j * sizeof(TimeStamp_t),
                     sizeof(TimeStamp_t));
      if (src1.file.fail()) {
        std::cerr << "Error reading from file: " << std::endl;
      }
    }
    /*****************************************************************/
    // file1의 frameHeader 쓰기
    dst.file.write(reinterpret_cast<char*>(file1_frmHead2d0),
                   sizeof(BinFrameHead2d0_t));

    // file1 timestamp 쓰기
    dst.file.write(timeStamp_Buffer1.get(), sizeof(TimeStamp_t));

    // file1, file2 data 찾기
    src1.file.seekg(src1.binHead.headerSz + src1.binHead.singleFrameSz * i);
    src2.file.seekg(src2.binHead.headerSz + src2.binHead.singleFrameSz * i);
    // file1, file2 data 읽기
    src1.file.read(dataBuffer1.get(), src1.binHead.singleFrameSz);
    src2.file.read(dataBuffer2.get(), src2.binHead.singleFrameSz);
    if (src2.file.fail()) {
      std::cerr << "Error reading from file: " << std::endl;
    }
    /*****************************************************************/
    // file1 data 쓰기
    dst.file.write(&dataBuffer1[sizeof(BinFrameHead2d0_t) +
                                sizeof(TimeStamp_t) * src1.lidarNum],
                   src1.binHead.singleFrameSz - sizeof(BinFrameHead2d0_t) -
                     sizeof(TimeStamp_t) * src1.lidarNum);

    // file2 data 쓰기(2.0ver)
    dst.file.write(&dataBuffer2[sizeof(BinFrameHead2d0_t)],
                   src2.binHead.singleFrameSz - sizeof(BinFrameHead2d0_t));
    /*****************************************************************/

    if (i % 20 == 0) {
      if (i % 400 == 0) {
        std::cout << std::endl;
      }
      std::cout << '#';
      flush(cout);
    }
  }
  dst.file.close();
  return newframeNum;
}

int second_ver2d1(BinFileInfo_t& src1, BinFileInfo_t& src2,
                  BinFileInfo_t& dst) {
  uint16_t min_frameNum = 0;
  std::unique_ptr<char[]> dataBuffer1(new char[src1.binHead.singleFrameSz]);
  std::unique_ptr<char[]> dataBuffer2(new char[src2.binHead.singleFrameSz]);

  std::unique_ptr<char[]> frameHeader_Buffer1(
    new char[sizeof(BinFrameHead2d0_t)]);
  std::unique_ptr<char[]> frameHeader_Buffer2(
    new char[sizeof(BinFrameHead2d0_t)]);

  std::unique_ptr<char[]> timeStamp_Buffer2(
    new char[sizeof(TimeStamp_t) * src1.lidarNum]);

  int newframeNum = 0;
  for (size_t i = 0; i < dst.frameNum; i++) {
    src1.file.seekg(src1.binHead.headerSz + src1.binHead.singleFrameSz * i);
    src2.file.seekg(src2.binHead.headerSz +
                    src2.binHead.singleFrameSz * min_frameNum);

    src1.file.read(frameHeader_Buffer1.get(), sizeof(BinFrameHead2d0_t));
    src2.file.read(frameHeader_Buffer2.get(), sizeof(BinFrameHead2d0_t));

    BinFrameHead2d0_t* file1_frmHead2d0 =
      reinterpret_cast<BinFrameHead2d0_t*>(frameHeader_Buffer1.get());

    BinFrameHead2d0_t* file2_frmHead2d0 =
      reinterpret_cast<BinFrameHead2d0_t*>(frameHeader_Buffer2.get());

    /*****************************************************************/
    uint32_t min_time_diff = 100000000;
    UNUSED(min_frameNum);
    UNUSED(min_time_diff);
    uint64_t time = 0;
    // time compare
    for (uint32_t file2_frameNum = 0; file2_frameNum < dst.frameNum;
         file2_frameNum++) {
      src2.file.seekg(src2.binHead.headerSz +
                      src2.binHead.singleFrameSz * (file2_frameNum));
      src2.file.read(frameHeader_Buffer2.get(), sizeof(BinFrameHead2d0_t));
      file2_frmHead2d0 =
        reinterpret_cast<BinFrameHead2d0_t*>(frameHeader_Buffer2.get());
      time = time_diff(file1_frmHead2d0, file2_frmHead2d0);
      if (min_time_diff > time) {
        min_time_diff = time;
        min_frameNum = file2_frameNum;
      }
    }

    if (min_time_diff >= 200) {
      continue;
    } else if (time == 0) {
      min_frameNum = i;
      continue;
    }
    newframeNum++;
    file1_frmHead2d0->frameNum = newframeNum;
    /*****************************************************************/

    // file2의 timeStamp 읽기
    if (min_frameNum == 0) {
      src2.file.seekg(src2.binHead.headerSz + src2.binHead.singleFrameSz * 0 +
                      sizeof(BinFrameHead2d0_t));
    } else {
      src2.file.seekg(src2.binHead.headerSz +
                      src2.binHead.singleFrameSz * (min_frameNum) +
                      sizeof(BinFrameHead2d0_t));
    }
    for (uint32_t j = 0; j < src1.lidarNum; j++) {
      src1.file.read(timeStamp_Buffer2.get() + j * sizeof(TimeStamp_t),
                     sizeof(TimeStamp_t));
      if (src1.file.fail()) {
        std::cerr << "Error reading from file: " << std::endl;
      }
    }
    /*****************************************************************/
    // file1의 frameHeader 쓰기
    dst.file.write(reinterpret_cast<char*>(file1_frmHead2d0),
                   sizeof(BinFrameHead2d0_t));

    // file1 timestamp 쓰기
    dst.file.write(timeStamp_Buffer2.get(), sizeof(TimeStamp_t));

    // file1, file2 data 찾기
    src1.file.seekg(src1.binHead.headerSz + src1.binHead.singleFrameSz * i);
    src2.file.seekg(src2.binHead.headerSz + src2.binHead.singleFrameSz * i);
    // file1, file2 data 읽기
    src1.file.read(dataBuffer1.get(), src1.binHead.singleFrameSz);
    src2.file.read(dataBuffer2.get(), src2.binHead.singleFrameSz);
    if (src2.file.fail()) {
      std::cerr << "Error reading from file: " << std::endl;
    }
    /*****************************************************************/

    // file1 data 쓰기(2.0ver)
    dst.file.write(&dataBuffer1[sizeof(BinFrameHead2d0_t)],
                   src1.binHead.singleFrameSz - sizeof(BinFrameHead2d0_t));

    // file2 data 쓰기
    dst.file.write(&dataBuffer2[sizeof(BinFrameHead2d0_t) +
                                sizeof(TimeStamp_t) * src2.lidarNum],
                   src2.binHead.singleFrameSz - sizeof(BinFrameHead2d0_t) -
                     sizeof(TimeStamp_t) * src2.lidarNum);
    /*****************************************************************/

    if (i % 20 == 0) {
      if (i % 400 == 0) {
        std::cout << std::endl;
      }
      std::cout << '#';
      flush(cout);
    }
  }
  dst.file.close();
  return newframeNum;
}

int mergeFrame2d0_timecomapre(BinFileInfo_t& src1, BinFileInfo_t& src2,
                              BinFileInfo_t& dst) {
  uint16_t min_frameNum = 0;

  std::unique_ptr<char[]> dataBuffer1(new char[src1.binHead.singleFrameSz]);
  std::unique_ptr<char[]> dataBuffer2(new char[src2.binHead.singleFrameSz]);

  std::unique_ptr<char[]> frameHeader_Buffer1(
    new char[sizeof(BinFrameHead2d0_t)]);
  std::unique_ptr<char[]> frameHeader_Buffer2(
    new char[sizeof(BinFrameHead2d0_t)]);

  check_file_length(frameHeader_Buffer1, frameHeader_Buffer2, src1, src2);
  int newframeNum = 0;
  for (size_t i = 0; i < dst.frameNum; i++) {
    src1.file.seekg(src1.binHead.headerSz + src1.binHead.singleFrameSz * i);
    src2.file.seekg(src2.binHead.headerSz +
                    src2.binHead.singleFrameSz * min_frameNum);
    src1.file.read(frameHeader_Buffer1.get(), sizeof(BinFrameHead2d0_t));
    src2.file.read(frameHeader_Buffer2.get(), sizeof(BinFrameHead2d0_t));

    BinFrameHead2d0_t* file1_frmHead2d0 =
      reinterpret_cast<BinFrameHead2d0_t*>(frameHeader_Buffer1.get());

    BinFrameHead2d0_t* file2_frmHead2d0 =
      reinterpret_cast<BinFrameHead2d0_t*>(frameHeader_Buffer2.get());

    uint32_t min_time_diff = 100000000;

    UNUSED(min_frameNum);
    UNUSED(min_time_diff);
    uint64_t time = 0;

    for (uint32_t file2_frameNum = 0; file2_frameNum < dst.frameNum;
         file2_frameNum++) {
      src2.file.seekg(src2.binHead.headerSz +
                      src2.binHead.singleFrameSz * (file2_frameNum));
      src2.file.read(frameHeader_Buffer2.get(), sizeof(BinFrameHead2d0_t));
      file2_frmHead2d0 =
        reinterpret_cast<BinFrameHead2d0_t*>(frameHeader_Buffer2.get());
      time = time_diff(file1_frmHead2d0, file2_frmHead2d0);
      if (min_time_diff > time) {
        min_time_diff = time;
        min_frameNum = file2_frameNum;
      }
    }

    if (min_time_diff >= 200) {
      continue;
      i--;
    }
    newframeNum++;
    file1_frmHead2d0->frameNum = newframeNum;
    dst.file.write(reinterpret_cast<char*>(file1_frmHead2d0),
                   sizeof(BinFrameHead2d0_t));
    // std::cout << "file2 min frame" << min_frameNum << std::endl;

    src1.file.seekg(src1.binHead.headerSz + src1.binHead.singleFrameSz * i);
    src2.file.seekg(src2.binHead.headerSz +
                    src2.binHead.singleFrameSz * min_frameNum);

    src1.file.read(dataBuffer1.get(), src1.binHead.singleFrameSz);
    src2.file.read(dataBuffer2.get(), src2.binHead.singleFrameSz);

    dst.file.write(&dataBuffer1[sizeof(BinFrameHead2d0_t)],
                   src1.binHead.singleFrameSz - sizeof(BinFrameHead2d0_t));
    dst.file.write(&dataBuffer2[sizeof(BinFrameHead2d0_t)],
                   src2.binHead.singleFrameSz - sizeof(BinFrameHead2d0_t));

    if (i % 20 == 0) {
      if (i % 400 == 0) {
        std::cout << std::endl;
      }
      std::cout << '#';
      flush(cout);
    }
  }
  dst.file.close();
  return newframeNum;
}

// 두파일 모두 Binver2.0 + forceOption이 false인 경우
int mergeBinFrame_timecompare(BinFileInfo_t& src1, BinFileInfo_t& src2,
                              BinFileInfo_t& dst) {
  int newframeNum = 0;
  if (src1.binHead.minorVer == BIN_MINOR_VER_1 &&
      src2.binHead.minorVer == BIN_MINOR_VER_1) {
    newframeNum = mergeFrame2d1_timecomapre(src1, src2, dst);
  } else {
    newframeNum = mergeFrame2d0_timecomapre(src1, src2, dst);
  }
  set_and_print_outputfile(dst, newframeNum);
  return 0;
}
int mergeFrame2d0_force(BinFileInfo_t& src1, BinFileInfo_t& src2,
                        BinFileInfo_t& dst) {
  BinFrameHead2d0_t frmHead2d0;
  uint16_t min_frameNum = 0;
  // BinFrameHead2d0_t* file1_frmHead2d0;
  // BinFrameHead2d0_t* file2_frmHead2d0;

  std::unique_ptr<char[]> dataBuffer1(new char[src1.binHead.singleFrameSz]);
  std::unique_ptr<char[]> dataBuffer2(new char[src2.binHead.singleFrameSz]);

  std::unique_ptr<char[]> frameHeader_Buffer1(
    new char[sizeof(BinFrameHead2d0_t)]);
  std::unique_ptr<char[]> frameHeader_Buffer2(
    new char[sizeof(BinFrameHead2d0_t)]);
  check_file_length(frameHeader_Buffer1, frameHeader_Buffer2, src1, src2);

  for (size_t i = 0; i < dst.frameNum; i++) {
    src1.file.seekg(src1.binHead.headerSz + src1.binHead.singleFrameSz * i);
    src2.file.seekg(src2.binHead.headerSz + src2.binHead.singleFrameSz * i);
    src1.file.read(frameHeader_Buffer1.get(), sizeof(BinFrameHead2d0_t));
    src2.file.read(frameHeader_Buffer2.get(), sizeof(BinFrameHead2d0_t));

    BinFrameHead2d0_t* file1_frmHead2d0 =
      reinterpret_cast<BinFrameHead2d0_t*>(frameHeader_Buffer1.get());

    [[maybe_unused]] BinFrameHead2d0_t* file2_frmHead2d0 =
      reinterpret_cast<BinFrameHead2d0_t*>(frameHeader_Buffer2.get());

    dst.file.write(reinterpret_cast<char*>(file1_frmHead2d0),
                   sizeof(BinFrameHead2d0_t));

    uint32_t min_time_diff = 100000000;

    UNUSED(min_frameNum);
    UNUSED(min_time_diff);

    src1.file.seekg(src1.binHead.headerSz + src1.binHead.singleFrameSz * i);
    src2.file.seekg(src2.binHead.headerSz + src2.binHead.singleFrameSz * i);

    src1.file.read(dataBuffer1.get(), src1.binHead.singleFrameSz);
    src2.file.read(dataBuffer2.get(), src2.binHead.singleFrameSz);

    dst.file.write(&dataBuffer1[sizeof(frmHead2d0)],
                   src1.binHead.singleFrameSz - sizeof(frmHead2d0));
    dst.file.write(&dataBuffer2[sizeof(frmHead2d0)],
                   src2.binHead.singleFrameSz - sizeof(frmHead2d0));

    if (i % 20 == 0) {
      if (i % 400 == 0) {
        std::cout << std::endl;
      }
      std::cout << '#';
      flush(cout);
    }
  }
  dst.file.close();
  set_and_print_outputfile(dst, dst.frameNum);

  return 0;
}

int mergeFrame2d1_force(BinFileInfo_t& src1, BinFileInfo_t& src2,
                        BinFileInfo_t& dst) {
  // BinFrameHead2d0_t* file1_frmHead2d0;
  // BinFrameHead2d0_t* file2_frmHead2d0;

  std::unique_ptr<char[]> dataBuffer1(new char[src1.binHead.singleFrameSz]);
  std::unique_ptr<char[]> dataBuffer2(new char[src2.binHead.singleFrameSz]);

  std::unique_ptr<char[]> frameHeader_Buffer1(
    new char[sizeof(BinFrameHead2d0_t)]);
  std::unique_ptr<char[]> frameHeader_Buffer2(
    new char[sizeof(BinFrameHead2d0_t)]);

  std::unique_ptr<char[]> timeStamp_Buffer1(
    new char[sizeof(TimeStamp_t) * src1.lidarNum]);
  std::unique_ptr<char[]> timeStamp_Buffer2(
    new char[sizeof(TimeStamp_t) * src2.lidarNum]);

  check_file_length(frameHeader_Buffer1, frameHeader_Buffer2, src1, src2);

  for (size_t i = 0; i < dst.frameNum; i++) {
    src1.file.seekg(src1.binHead.headerSz + src1.binHead.singleFrameSz * i);
    src2.file.seekg(src2.binHead.headerSz + src2.binHead.singleFrameSz * i);
    src1.file.read(frameHeader_Buffer1.get(), sizeof(BinFrameHead2d0_t));
    src2.file.read(frameHeader_Buffer2.get(), sizeof(BinFrameHead2d0_t));

    BinFrameHead2d0_t* file1_frmHead2d0 =
      reinterpret_cast<BinFrameHead2d0_t*>(frameHeader_Buffer1.get());

    [[maybe_unused]] BinFrameHead2d0_t* file2_frmHead2d0 =
      reinterpret_cast<BinFrameHead2d0_t*>(frameHeader_Buffer2.get());

    dst.file.write(reinterpret_cast<char*>(file1_frmHead2d0),
                   sizeof(BinFrameHead2d0_t));

    src1.file.seekg(src1.binHead.headerSz + src1.binHead.singleFrameSz * i +
                    sizeof(BinFrameHead2d0_t));
    for (uint32_t j = 0; j < src1.lidarNum; j++) {
      src1.file.read(timeStamp_Buffer1.get() + j * sizeof(TimeStamp_t),
                     sizeof(TimeStamp_t));
      if (src1.file.fail()) {
        std::cerr << "Error reading from file: " << std::endl;
      }
    }
    src2.file.seekg(src2.binHead.headerSz + src2.binHead.singleFrameSz * i +
                    sizeof(BinFrameHead2d0_t));

    for (uint32_t j = 0; j < src2.lidarNum; j++) {
      src2.file.read(timeStamp_Buffer2.get() + j * sizeof(TimeStamp_t),
                     sizeof(TimeStamp_t));
      if (src2.file.fail()) {
        std::cerr << "Error reading from file: " << std::endl;
      }
    }
    dst.file.write(timeStamp_Buffer1.get(), sizeof(TimeStamp_t));
    dst.file.write(timeStamp_Buffer2.get(), sizeof(TimeStamp_t));

    src1.file.seekg(src1.binHead.headerSz + src1.binHead.singleFrameSz * i);
    src2.file.seekg(src2.binHead.headerSz + src2.binHead.singleFrameSz * i);

    src1.file.read(dataBuffer1.get(), src1.binHead.singleFrameSz);
    src2.file.read(dataBuffer2.get(), src2.binHead.singleFrameSz);

    dst.file.write(&dataBuffer1[sizeof(BinFrameHead2d0_t) +
                                sizeof(TimeStamp_t) * src1.lidarNum],
                   src1.binHead.singleFrameSz - sizeof(BinFrameHead2d0_t) -
                     sizeof(TimeStamp_t) * src1.lidarNum);

    dst.file.write(&dataBuffer2[sizeof(BinFrameHead2d0_t) +
                                sizeof(TimeStamp_t) * src2.lidarNum],
                   src2.binHead.singleFrameSz - sizeof(BinFrameHead2d0_t) -
                     sizeof(TimeStamp_t) * src2.lidarNum);

    if (i % 20 == 0) {
      if (i % 400 == 0) {
        std::cout << std::endl;
      }
      std::cout << '#';
      flush(cout);
    }
  }
  dst.file.close();
  set_and_print_outputfile(dst, dst.frameNum);

  return 0;
}

int mergeFrame2d1_to2d0_force(BinFileInfo_t& src1, BinFileInfo_t& src2,
                              BinFileInfo_t& dst) {
  // BinFrameHead2d0_t* file1_frmHead2d0;
  BinFrameHead2d0_t frmHead2d0;

  std::unique_ptr<char[]> dataBuffer1(new char[src1.binHead.singleFrameSz]);
  std::unique_ptr<char[]> dataBuffer2(new char[src2.binHead.singleFrameSz]);

  std::unique_ptr<char[]> frameHeader_Buffer1(
    new char[sizeof(BinFrameHead2d0_t)]);
  std::unique_ptr<char[]> frameHeader_Buffer2(
    new char[sizeof(BinFrameHead2d0_t)]);

  std::unique_ptr<char[]> timeStamp_Buffer1(
    new char[sizeof(TimeStamp_t) * src1.lidarNum]);

  check_file_length(frameHeader_Buffer1, frameHeader_Buffer2, src1, src2);

  for (size_t i = 0; i < dst.frameNum; i++) {
    memset(&frmHead2d0, 0, sizeof(frmHead2d0));
    strcpy(frmHead2d0.frameMarker, FRAME_MARKER);
    frmHead2d0.frameNum = i;
    sprintf(frmHead2d0.str, "#%d", frmHead2d0.frameNum);

    dst.file.write((char*)&frmHead2d0, sizeof(frmHead2d0));

    src1.file.seekg(src1.binHead.headerSz + src1.binHead.singleFrameSz * i);
    src2.file.seekg(src2.binHead.headerSz + src2.binHead.singleFrameSz * i);

    src1.file.read(dataBuffer1.get(), src1.binHead.singleFrameSz);
    src2.file.read(dataBuffer2.get(), src2.binHead.singleFrameSz);

    dst.file.write(&dataBuffer1[sizeof(BinFrameHead2d0_t) +
                                sizeof(TimeStamp_t) * src1.lidarNum],
                   src1.binHead.singleFrameSz - sizeof(BinFrameHead2d0_t) -
                     sizeof(TimeStamp_t) * src1.lidarNum);

    dst.file.write(&dataBuffer2[sizeof(BinFrameHead2d0_t)],
                   src2.binHead.singleFrameSz - sizeof(BinFrameHead2d0_t));

    if (i % 20 == 0) {
      if (i % 400 == 0) {
        std::cout << std::endl;
      }
      std::cout << '#';
      flush(cout);
    }
  }
  dst.file.close();
  set_and_print_outputfile(dst, dst.frameNum);

  return 0;
}

// 두파일 모두 Binver2.0 + forceOption이 true인 경우
int mergeBinFrame_timecompare_force(BinFileInfo_t& src1, BinFileInfo_t& src2,
                                    BinFileInfo_t& dst) {
  int newframeNum = 0;
  if (src1.binHead.minorVer == BIN_MINOR_VER_1 &&
      src2.binHead.minorVer == BIN_MINOR_VER_1) {
    newframeNum = mergeFrame2d1_force(src1, src2, dst);
  } else if ((src1.binHead.minorVer == BIN_MINOR_VER_1 &&
              src2.binHead.minorVer != BIN_MINOR_VER_1) ||
             (src1.binHead.minorVer != BIN_MINOR_VER_1 &&
              src2.binHead.minorVer == BIN_MINOR_VER_1)) {
    newframeNum = mergeFrame2d1_to2d0_force(src1, src2, dst);

  } else {
    newframeNum = mergeFrame2d0_force(src1, src2, dst);
  }
  return newframeNum;
}

int main(int argc, char* argv[]) {
  checkOpt(f1.fileName, f2.fileName, argc, argv);

  if (loadBinHeader(f1) || loadBinHeader(f2)) {
    cout << "Fail to Load File" << endl;
    return 1;
  }

  checkBinVersion(f1);
  checkBinVersion(f2);

  if ((f1.binHead.majorVer != f2.binHead.majorVer) ||
      (f1.binHead.minorVer != f2.binHead.minorVer)) {
    printWarning("major ver not match || merge forced");
    forceOption = true;
  }

  if (f1.binHead.majorVer != BIN_MAJOR_VER_2 ||
      f2.binHead.majorVer != BIN_MAJOR_VER_2) {
    mergeBinHeader(f1, f2, target);
    mergeBinFrame(f1, f2, target);
  } else if (f1.binHead.majorVer == BIN_MAJOR_VER_2 &&
             f2.binHead.majorVer == BIN_MAJOR_VER_2) {
    if (forceOption) {
      mergeBinHeader(f1, f2, target);
      mergeBinFrame_timecompare_force(f1, f2, target);
    } else if (!forceOption) {
      mergeBinHeader(f1, f2, target);
      mergeBinFrame_timecompare(f1, f2, target);
    }
  }

  return 0;
}