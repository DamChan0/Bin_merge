#include "fileControl.h"

#define OPT_FMT "1:2:3:4:5:6:7:8:9:fts:"

using namespace MergeLidarData;

template <typename... Args>
void coutDebug(const std::string& message, Args... args) {
  auto now = std::chrono::system_clock::now();
  std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

  std::cout << std::put_time(std::localtime(&currentTime), "%Y-%m-%d %X")
            << " ";
  std::cout << ANSI_COLOR_GREEN << "[DEBUG]" << ANSI_COLOR_RESET << " ";
  // 가변 인자 처리
  std::ostringstream ss;
  (void)std::initializer_list<int>{(ss << std::forward<Args>(args), 0)...};

  // 형식화된 문자열 출력
  std::cout << ANSI_COLOR_GREEN << message;
  std::cout << ANSI_COLOR_GREEN << " " << ss.str();
  std::cout << std::endl;
  std::cout << "\033[0m";
}
template <typename... Args>
void coutInfo(const std::string& message, Args... args) {
  auto now = std::chrono::system_clock::now();
  std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

  std::cout << std::put_time(std::localtime(&currentTime), "%Y-%m-%d %X")
            << " ";
  std::cout << ANSI_COLOR_PURPLE << "[INFO]" << ANSI_COLOR_RESET << " ";
  // 가변 인자 처리
  std::ostringstream ss;
  (void)std::initializer_list<int>{(ss << std::forward<Args>(args), 0)...};

  // 형식화된 문자열 출력
  std::cout << ANSI_COLOR_PURPLE << message;
  std::cout << ANSI_COLOR_PURPLE << " " << ss.str();
  std::cout << endl;
  std::cout << "\033[0m";
}

uint32_t MergeLidarData::updata_File::checkOpt(int argc,
                                               std::vector<std::string> argv,
                                               uint32_t* option,
                                               uint32_t splitstart,
                                               uint32_t splitEnd) {
  fileInfo = std::make_unique<BinFileInfo[]>(10);

  if (argc == 1 && *option != 3) {
    cout << "Error: Argument Input Error" << endl;
    return 1;
  }
  int fileIndex = 0;  // 파일을 저장할 배열의 인덱스
  for (size_t i = 0; i < argv.size(); ++i) {
    fileInfo[i].filePath = argv[i];
    fileIndex++;
  }

  if (*option == 3) {
    splitNum_start = splitstart;
    splitNum_end = splitEnd;
  }
  mergeOption = *option;
  return fileIndex;
}

void MergeLidarData::updata_File::checkHeaderInfo(uint32_t fileIndex) {
  for (uint32_t i = 0; i < fileIndex; i++) {
    // Print the current working directory
    std::fstream fileStream;

    fileStream.open(fileInfo[i].filePath,
                    std::ios_base::in | std::ios_base::binary);

    if (fileStream.is_open()) {
      fileStream.read((char*)&fileInfo[i].binHead, sizeof(BinHeaderInfo));
      fileInfo[i].lidarNum = fileInfo[i].binHead.sensorCnt;

      if (!fileStream) {
        // Check for errors
        std::cerr << "Error reading data from file:" << fileInfo[i].filePath
                  << std::endl;
      } else {
        coutDebug("Success open file", fileInfo[i].filePath);
      }

      std::cout << "file Version:" << fileInfo[i].binHead.majorVer << "."
                << fileInfo[i].binHead.minorVer << std::endl;

      fileStream.close();
    } else {
      std::cerr << "Error opening file: " << fileInfo[i].filePath << std::endl;
    }
  }
}

bool MergeLidarData::updata_File::read_File_Header(uint32_t fileIndex) {
  std::streamoff ofs = sizeof(BinHeaderInfo);
  uint32_t lidarNum = 0;
  std::ifstream fileStream;

  for (uint32_t i = 0; i < fileIndex; i++) {
    fileInfo[i].file.open(fileInfo[i].filePath,
                          std::ios_base::in | std::ios_base::binary);
    fileInfo[i].lidarNum = fileInfo[i].binHead.sensorCnt;

    fileInfo[i].size = std::filesystem::file_size(fileInfo[i].filePath);

    fileInfo[i].sensors =
      std::make_unique<BinHeadSensor[]>(fileInfo[i].binHead.calCnt);

    fileInfo[i].cal =
      std::make_unique<BinHeadCal[]>(fileInfo[i].binHead.calCnt);

    fileInfo[i].file.seekg(ofs);

    for (uint32_t j = 0; j < fileInfo[i].binHead.calCnt; j++) {
      fileInfo[i].file.read((char*)&fileInfo[i].sensors[j],
                            sizeof(BinHeadSensor));

      std::fill(
        fileInfo[i].sensors[j].subId,
        fileInfo[i].sensors[j].subId + sizeof(fileInfo[i].sensors[j].subId), 0);

      lidarNum++;
      std::string newSubId = "#" + std::to_string(lidarNum);
      std::copy(newSubId.begin(), newSubId.end(), fileInfo[i].sensors[j].subId);

      if (!fileInfo[i].file) {
        std::cerr << "파일 읽기 오류 발생." << std::endl;
        goto ERR;  // 오류 코드 반환
      }

      coutInfo("framesz:", "file:", i + 1, "번 ", "Lidar:", j + 1, "번 ",
               fileInfo[i].sensors[j].str);
      coutInfo("Lidar subid:", fileInfo[i].sensors[j].subId);
    }

    for (uint32_t j = 0; j < fileInfo[i].binHead.calCnt; j++) {
      fileInfo[i].file.read((char*)&fileInfo[i].cal[j], sizeof(BinHeadCal));

      if (!fileInfo[i].file) {
        std::cerr << "파일 읽기 오류 발생." << std::endl;
        goto ERR;  // 오류 코드 반환
      }
      std::cout << "filepointer:" << fileInfo[i].file.tellg() << std::endl;
      coutInfo("framesz:", "file:", i + 1, "번 ", "Lidar:", j + 1, "번 ",
               fileInfo[i].cal[j].str);
    }

    fileInfo[i].frameNum = (fileInfo[i].size - fileInfo[i].binHead.headerSz) /
                           fileInfo[i].binHead.singleFrameSz;

    fileInfo[i].majorVer = (eBinMajorVersion)fileInfo[i].binHead.majorVer;
    fileInfo[i].minorVer = (eBinMinorVersion)fileInfo[i].binHead.minorVer;
  }
  return 1;

ERR:
  return -1;
}

bool MergeLidarData::updata_File::merge_Header(uint32_t fileIndex) {
  outPutFile = std::make_unique<BinFileInfo>();

  std::ostringstream fileNameStream;
  std::string outputFilePath;
  std::string baseName = "merged";
  std::string baseNameSplit = "splited_file";
  if (mergeOption == 1 || mergeOption == 2) {
    fileNameStream << baseName;
    std::time_t t = std::time(nullptr);
    std::tm* timelocal = std::localtime(&t);

    fileNameStream << "_" << (timelocal->tm_year + 1900) << std::setw(2)
                   << std::setfill('0') << (timelocal->tm_mon + 1)
                   << std::setw(2) << std::setfill('0') << timelocal->tm_mday
                   << std::setw(2) << std::setfill('0') << timelocal->tm_hour
                   << std::setw(2) << std::setfill('0') << timelocal->tm_min
                   << std::setw(2) << std::setfill('0') << timelocal->tm_sec;

    fileNameStream << ".bin";

    outputFilePath = fileNameStream.str();
  } else {
    std::filesystem::path path(fileInfo[0].filePath);
    std::string fileName = path.filename().string();
    fileNameStream << baseNameSplit;
    fileNameStream << fileName;
    // fileNameStream << ".bin";
    outputFilePath = fileNameStream.str();
  }

  uint32_t totalLidar = 0;
  for (uint32_t i = 0; i < fileIndex; i++) {
    totalLidar = totalLidar + fileInfo[i].lidarNum;
  }

  outPutFile->sensors = make_unique<BinHeadSensor[]>(totalLidar);
  outPutFile->cal = make_unique<BinHeadCal[]>(totalLidar);
  std::cout << outputFilePath << std::endl;
  outPutFile.get()->filePath = outputFilePath;  // filrPath
  outPutFile.get()->file.open(
    outputFilePath,
    std::ios_base::out | std::ios_base::binary);  // fstream

  outPutFile.get()->lidarNum = totalLidar;  // lidarNum

  outPutFile->sensors = make_unique<BinHeadSensor[]>(totalLidar);
  outPutFile->cal = make_unique<BinHeadCal[]>(totalLidar);

  outPutFile.get()->frameNum = calcFrameNum(fileIndex);  // frameSz

  /******************************binheaderinfo********************************/
  memcpy(outPutFile.get()->binHead.marker, fileInfo[0].binHead.marker,
         8);  // marker

  if (mergeOption == 3) {
    outPutFile.get()->binHead.majorVer = fileInfo.get()->majorVer;  // major
    outPutFile.get()->binHead.minorVer = fileInfo.get()->minorVer;  // minor
  }
  outPutFile.get()->binHead.headerSz =
    sizeof(BinHeaderInfo) + totalLidar * sizeof(BinHeadSensor) +
    totalLidar * sizeof(BinHeadCal);                 // headerSz
  outPutFile.get()->binHead.sensorCnt = totalLidar;  // sensorcnt
  outPutFile.get()->binHead.calCnt = totalLidar;     // calcnt

  /******************************singleframeSz********************************/
  outPutFile.get()->binHead.singleFrameSz = 0;  //
  uint32_t duplicatedFrameHeader = calcSingleframeSize(fileIndex);

  outPutFile.get()->majorVer =
    (eBinMajorVersion)outPutFile.get()->binHead.majorVer;  // majorver
  outPutFile.get()->minorVer =
    (eBinMinorVersion)outPutFile.get()->binHead.minorVer;  // minor
  for (uint32_t i = 0; i < fileIndex; i++) {
    outPutFile.get()->binHead.singleFrameSz +=
      fileInfo[i].binHead.singleFrameSz;
  }
  outPutFile.get()->binHead.singleFrameSz -= duplicatedFrameHeader;
  /**************************************************************************/

  /******************************headerStr********************************/
  std::sprintf(
    outPutFile.get()->binHead.str,
    "Ver:%02d.%02d, hd_sz: %d, senCnt:%d, calCnt:%d,singleFrmSz:%d",
    outPutFile.get()->binHead.majorVer, outPutFile.get()->binHead.minorVer,
    outPutFile.get()->binHead.headerSz, outPutFile.get()->binHead.sensorCnt,
    outPutFile.get()->binHead.calCnt, outPutFile.get()->binHead.singleFrameSz);
  /**************************************************************************/

  uint32_t senseorNum = 0;
  /******************************mergeSensorHeader*******************************/
  for (uint32_t i = 0; i < fileIndex; i++) {
    for (uint32_t j = 0; j < fileInfo[i].lidarNum; j++) {
      std::memcpy(&outPutFile.get()->sensors[senseorNum],
                  &fileInfo[i].sensors[j], sizeof(BinHeadSensor));
      senseorNum++;
    }
  }
  /**************************************************************************/
  senseorNum = 0;
  /******************************mergeCalHeader*******************************/

  for (uint32_t i = 0; i < fileIndex; i++) {
    for (uint32_t j = 0; j < fileInfo[i].lidarNum; j++) {
      std::memcpy(&outPutFile.get()->cal[senseorNum], &fileInfo[i].cal[j],
                  sizeof(BinHeadCal));
      senseorNum++;
    }
  }
  /**************************************************************************/

  senseorNum = 0;

  /**************************************************************************/
  outPutFile.get()->file.write((char*)&outPutFile.get()->binHead,
                               sizeof(outPutFile.get()->binHead));
  outPutFile.get()->file.flush();

  for (uint32_t j = 0; j < outPutFile.get()->lidarNum; j++) {
    outPutFile.get()->file.write((char*)&outPutFile.get()->sensors[j],
                                 sizeof(BinHeadSensor));
  }
  outPutFile.get()->file.flush();

  for (uint32_t j = 0; j < outPutFile.get()->lidarNum; j++) {
    outPutFile.get()->file.write((char*)&outPutFile.get()->cal[j],
                                 sizeof(BinHeadCal));
  }
  outPutFile.get()->file.flush();

  /**************************************************************************/

  return 1;
}

uint32_t MergeLidarData::updata_File::calcFrameNum(uint32_t fileIndex) {
  auto minFrameIterator =
    std::min_element(fileInfo.get(), fileInfo.get() + fileIndex,
                     [](const BinFileInfo& a, const BinFileInfo& b) {
                       return a.frameNum < b.frameNum;
                     });

  return (minFrameIterator != fileInfo.get() + fileIndex)
           ? minFrameIterator->frameNum
           : 0;
}

uint32_t MergeLidarData::updata_File::calcSingleframeSize(uint32_t fileIndex) {
  if (fileIndex == 2) {
    // 1.2 - 1.2
    if (fileInfo[0].majorVer == 1 && fileInfo[1].majorVer == 1) {
      printDebug("BinfileInfo[0].binHead.majorVer + Binsrc1.binHead.majorVer",
                 "BinVersion: ");
      outPutFile.get()->binHead.majorVer = 1;
      outPutFile.get()->binHead.minorVer = 2;
      return sizeof(BinFrameHead);
    }
    // 1.2 - 2.1 = 2.0
    else if (fileInfo[0].majorVer == 1 && fileInfo[1].majorVer == 2 &&
             fileInfo[1].binHead.minorVer == 1) {
      printDebug("Binsrc1.binHead.majorVer + Binsrc1.binHead.majorVer",
                 "BinVersion: ");
      outPutFile.get()->binHead.majorVer = 2;
      outPutFile.get()->binHead.minorVer = 0;
      return sizeof(BinFrameHead) + sizeof(TimeStamp) * fileInfo[1].lidarNum;
    }
    // 2.1 - 1.2 = 2.0
    else if (fileInfo[1].majorVer == 1 && fileInfo[0].majorVer == 2 &&
             fileInfo[0].binHead.minorVer == 1) {
      printDebug("Binsrc1.binHead.majorVer + Binsrc1.binHead.majorVer",
                 "BinVersion: ");
      outPutFile.get()->binHead.majorVer = 2;
      outPutFile.get()->binHead.minorVer = 0;
      return sizeof(BinFrameHead) + sizeof(TimeStamp) * fileInfo[0].lidarNum;
    }
    // 1.2 - 2.0 = 2.0
    else if (fileInfo[0].majorVer == 1 && fileInfo[1].majorVer == 2 &&
             fileInfo[1].binHead.minorVer == 0) {
      printDebug("Binsrc1.binHead.majorVer + Binsrc1.binHead.majorVer",
                 "BinVersion: ");
      outPutFile.get()->binHead.majorVer = 2;
      outPutFile.get()->binHead.minorVer = 0;
      return sizeof(BinFrameHead);
    }
    // 2.0 - 1.2 = 2.0
    else if (fileInfo[1].majorVer == 1 && fileInfo[0].majorVer == 2 &&
             fileInfo[0].binHead.minorVer == 0) {
      printDebug("Binsrc1.binHead.majorVer + Binsrc1.binHead.majorVer",
                 "BinVersion: ");
      outPutFile.get()->binHead.majorVer = 2;
      outPutFile.get()->binHead.minorVer = 0;
      return sizeof(BinFrameHead);
    }
    // 2.0 - 2.1 = 2.1
    else if (fileInfo[1].majorVer == 2 && fileInfo[0].majorVer == 2 &&
             fileInfo[1].binHead.minorVer == 0 &&
             fileInfo[0].binHead.minorVer == 1) {
      printDebug("Binsrc1.binHead.majorVer + Binsrc1.binHead.majorVer",
                 "BinVersion: ");
      outPutFile.get()->binHead.majorVer = 2;
      outPutFile.get()->binHead.minorVer = 1;
      return sizeof(BinFrameHead2d0) -
             (sizeof(TimeStamp) * fileInfo[0].lidarNum);
    }
    // 2.1 - 2.0 = 2.1
    else if (fileInfo[1].majorVer == 2 && fileInfo[0].majorVer == 2 &&
             fileInfo[1].binHead.minorVer == 1 &&
             fileInfo[0].binHead.minorVer == 0) {
      printDebug("Binsrc1.binHead.majorVer + Binsrc1.binHead.majorVer",
                 "BinVersion: ");
      outPutFile.get()->binHead.majorVer = 2;
      outPutFile.get()->binHead.minorVer = 1;
      return sizeof(BinFrameHead2d0) -
             (sizeof(TimeStamp) * fileInfo[1].lidarNum);
    }
    // 2.0 - 2.0 = 2.0
    else if (fileInfo[1].majorVer == 2 && fileInfo[0].majorVer == 2 &&
             fileInfo[1].binHead.minorVer == 0 &&
             fileInfo[0].binHead.minorVer == 0) {
      printDebug("Binsrc1.binHead.majorVer + Binsrc1.binHead.majorVer",
                 "BinVersion: ");
      outPutFile.get()->binHead.majorVer = 2;
      outPutFile.get()->binHead.minorVer = 0;
      return sizeof(BinFrameHead2d0);
    }
    // 2.1 - 2.1 = 2.1  ctiem compare by frame timestamp / Do not using lidar
    // timestamp
    else if (fileInfo[1].majorVer == 2 && fileInfo[0].majorVer == 2 &&
             fileInfo[1].binHead.minorVer == 1 &&
             fileInfo[0].binHead.minorVer == 1) {
      if (mergeOption != 2) {
        printDebug("Binsrc1.binHead.majorVer + Binsrc1.binHead.majorVer",
                   "BinVersion: ");
        outPutFile.get()->binHead.majorVer = 2;
        outPutFile.get()->binHead.minorVer = 1;
        return sizeof(BinFrameHead2d0);
      } else {
        printDebug("Binsrc1.binHead.majorVer + Binsrc1.binHead.majorVer",
                   "BinVersion: ");
        outPutFile.get()->binHead.majorVer = 2;
        outPutFile.get()->binHead.minorVer = 1;
        return sizeof(BinFrameHead2d0);
      }
    }

  }
  // 3개 이상의 파일 merge는 2.1 버전만 지원 = > 2.0 으로 변경
  else if (fileIndex >= 3) {
    uint32_t result = 0;
    uint32_t multiMergeValid = 0;
    for (uint32_t i = 0; i < fileIndex; i++) {
      multiMergeValid++;
    }
    if (multiMergeValid != fileIndex)
      printError("merging more than two is supported only V2.1");
    else {
      coutDebug("merge Valid");
    }
    // for (uint32_t i = 0; i < fileIndex; i++) {
    //   result += fileInfo[i].lidarNum * sizeof(TimeStamp);
    // }
    result += sizeof(BinFrameHead2d0) * (fileIndex - 1);

    outPutFile.get()->binHead.majorVer = 2;
    outPutFile.get()->binHead.minorVer = 1;
    return result;
  }
  return 0;
}
void MergeLidarData::updata_File::insertFrame_2d1(uint32_t splitNum_start,
                                                  uint32_t splitNum_end) {
  std::unique_ptr<char[]> dataBuffer =
    std::make_unique<char[]>(outPutFile.get()->binHead.singleFrameSz);

  std::unique_ptr<BinFrameHead2d0> frameHeaderBuffer =
    std::make_unique<BinFrameHead2d0>();

  std::unique_ptr<char[]> timeStampBuffer =
    std::make_unique<char[]>(fileInfo.get()->lidarNum * sizeof(TimeStamp));

  if (fileInfo.get()->file.is_open()) {
    coutDebug("file opened");
  }
  for (uint64_t i = splitNum_start; i < splitNum_end; i++) {
    std::streamoff offs;
    offs = (static_cast<long long>(fileInfo.get()->binHead.headerSz) +
            static_cast<long long>(fileInfo.get()->binHead.singleFrameSz * i));
    fileInfo.get()->file.seekg(offs);
    // offs = 4.0 * 1024.0 * 1024.0 * 1024.0 * 1024.0;
    cout << "offs : " << offs << endl;
    cout << "frameNum : " << i << endl;
    fileInfo.get()->file.read((char*)frameHeaderBuffer.get(),
                              sizeof(BinFrameHead2d0));
    outPutFile.get()->file.write((char*)frameHeaderBuffer.get(),
                                 sizeof(BinFrameHead2d0));
    outPutFile.get()->file.flush();

    for (uint32_t j = 0; j < fileInfo.get()->lidarNum; j++) {
      fileInfo.get()->file.read(timeStampBuffer.get(), sizeof(TimeStamp));

      outPutFile.get()->file.write(timeStampBuffer.get(), sizeof(TimeStamp));
      outPutFile.get()->file.flush();
    }

    fileInfo.get()->file.read(dataBuffer.get(),
                              fileInfo.get()->binHead.singleFrameSz -
                                sizeof(BinFrameHead2d0) -
                                sizeof(TimeStamp) * fileInfo.get()->lidarNum);

    std::cout << "file pointer " << fileInfo.get()->file.tellg() << std::endl;

    outPutFile.get()->file.write(
      dataBuffer.get(), fileInfo.get()->binHead.singleFrameSz -
                          sizeof(BinFrameHead2d0) -
                          sizeof(TimeStamp) * fileInfo.get()->lidarNum);
    outPutFile.get()->file.flush();
  }
}

void MergeLidarData::updata_File::insertFrame_2d0(uint32_t splitNum_start,
                                                  uint32_t splitNum_end) {
  std::unique_ptr<char[]> dataBuffer =
    std::make_unique<char[]>(outPutFile.get()->binHead.singleFrameSz);

  std::unique_ptr<BinFrameHead2d0> frameHeaderBuffer =
    std::make_unique<BinFrameHead2d0>();

  if (fileInfo.get()->file.is_open()) {
    coutDebug("file opened");
  }
  for (uint32_t i = splitNum_start; i < splitNum_end; i++) {
    fileInfo.get()->file.seekg(fileInfo.get()->binHead.headerSz +
                               fileInfo.get()->binHead.singleFrameSz * i);

    fileInfo.get()->file.read((char*)frameHeaderBuffer.get(),
                              sizeof(BinFrameHead2d0));
    outPutFile.get()->file.write((char*)frameHeaderBuffer.get(),
                                 sizeof(BinFrameHead2d0));
    outPutFile.get()->file.flush();

    fileInfo.get()->file.read(
      dataBuffer.get(),
      fileInfo.get()->binHead.singleFrameSz - sizeof(BinFrameHead2d0));

    std::cout << "file pointer " << fileInfo.get()->file.tellg() << std::endl;

    outPutFile.get()->file.write(
      dataBuffer.get(),
      fileInfo.get()->binHead.singleFrameSz - sizeof(BinFrameHead2d0));
    outPutFile.get()->file.flush();
  }
}

uint32_t MergeLidarData::updata_File::split(uint32_t splitNum_start,
                                            uint32_t splitNum_end) {
  if (fileInfo.get()->majorVer == 1) {
    for (uint32_t i = splitNum_start; i <= splitNum_end; i++) {
      outPutFile.get()->file.write((char*)&fileInfo.get()->file,
                                   sizeof(BinFrameHead));
      outPutFile.get()->file.write(
        (char*)&fileInfo.get()->file,
        sizeof(fileInfo.get()->binHead.singleFrameSz - sizeof(BinFrameHead)));
    }
  } else {
    if (fileInfo.get()->binHead.minorVer == 1) {
      insertFrame_2d1(splitNum_start, splitNum_end);

    } else if (fileInfo.get()->binHead.minorVer == 0) {
      insertFrame_2d0(splitNum_start, splitNum_end);
    }
  }
  outPutFile.get()->file.close();

  return 1;
}

bool MergeLidarData::updata_File::merge_Frame(uint32_t startFrmae,
                                              uint32_t endFrmae,
                                              uint32_t fileIndex) {
  BinFrameHead2d0 frmHead2d0;

  for (uint64_t i = startFrmae; i <= endFrmae; i++) {
    if (i == 531) {
      int a = 0;
      UNUSED(a);
    }
    if (i == 76) {
      int a = 0;
      UNUSED(a);
    }

    memset(&frmHead2d0, 0, sizeof(frmHead2d0));
    strcpy(frmHead2d0.frameMarker, FRAME_MARKER);
    frmHead2d0.frameNum = i;
    sprintf(frmHead2d0.str, "#%d", frmHead2d0.frameNum);
    outPutFile.get()->file.write((char*)&frmHead2d0, sizeof(frmHead2d0));
    outPutFile.get()->file.flush();

    for (uint64_t j = 0; j < fileIndex; j++) {
      if (!fileInfo[j].file.is_open()) {
        cout << "file error" << endl;
      }
      std::unique_ptr<TimeStamp> timeStampBuffer =
        std::make_unique<TimeStamp>();
      fileInfo[j].file.clear();
      fileInfo[j].file.seekg(fileInfo[j].binHead.headerSz +
                             fileInfo[j].binHead.singleFrameSz * i +
                             sizeof(BinFrameHead2d0));
      cout << "file pointer : "
           << "fileNum: " << j << "fileFrmae: " << i << "__"
           << fileInfo[j].file.tellg() << endl;
      if (fileInfo[j].majorVer == BIN_MAJOR_VER_2 &&
          fileInfo[j].minorVer == BIN_MINOR_VER_1) {
        for (uint32_t t = 0; t < fileInfo[j].lidarNum; t++) {
          fileInfo[j].file.read((char*)timeStampBuffer.get(),
                                sizeof(TimeStamp));
          outPutFile.get()->file.write((char*)timeStampBuffer.get(),
                                       sizeof(TimeStamp));
          outPutFile.get()->file.flush();
        }

      } else if (fileInfo[j].majorVer == BIN_MAJOR_VER_2 &&
                 fileInfo[j].minorVer == BIN_MINOR_VER_NONE) {
        for (uint64_t t = 0; t < fileInfo[j].lidarNum; t++) {
          TimeStamp timeStamp;
          memset(&timeStamp, 0, sizeof(timeStamp));
          outPutFile.get()->file.write((char*)&timeStamp, sizeof(timeStamp));
          outPutFile.get()->file.flush();
        }
      }
    }

    for (uint64_t j = 0; j < fileIndex; j++) {
      std::unique_ptr<char[]> dataBuffer =
        std::make_unique<char[]>(outPutFile.get()->binHead.singleFrameSz);

      if (fileInfo[j].majorVer == BIN_MAJOR_VER_2 &&
          fileInfo[j].minorVer == BIN_MINOR_VER_1) {
        fileInfo[j].file.seekg(
          fileInfo[j].binHead.headerSz + fileInfo[j].binHead.singleFrameSz * i +
          sizeof(BinFrameHead2d0) + sizeof(TimeStamp) * fileInfo[j].lidarNum);

        fileInfo[j].file.read(dataBuffer.get(),
                              fileInfo[j].binHead.singleFrameSz -
                                (sizeof(BinFrameHead2d0) +
                                 sizeof(TimeStamp) * fileInfo[j].lidarNum));

        outPutFile.get()->file.write(
          dataBuffer.get(), fileInfo[j].binHead.singleFrameSz -
                              sizeof(BinFrameHead2d0) -
                              sizeof(TimeStamp) * fileInfo[j].lidarNum);

        outPutFile.get()->file.flush();
      }

      else if (fileInfo[j].majorVer == BIN_MAJOR_VER_2 &&
               fileInfo[j].minorVer == BIN_MINOR_VER_NONE) {
        fileInfo[j].file.seekg(fileInfo[j].binHead.headerSz +
                               fileInfo[j].binHead.singleFrameSz * i +
                               sizeof(BinFrameHead2d0));

        fileInfo[j].file.read(
          dataBuffer.get(),
          fileInfo[j].binHead.singleFrameSz - sizeof(BinFrameHead2d0));

        outPutFile.get()->file.write(
          dataBuffer.get(),
          fileInfo[j].binHead.singleFrameSz - sizeof(BinFrameHead2d0));

        outPutFile.get()->file.flush();
      }
    }
  }
  outPutFile.get()->file.close();

  return 1;
}

bool MergeLidarData::updata_File::merge_Frame_Timecompare(uint32_t startFrmae,
                                                          uint32_t endFrmae,
                                                          uint32_t fileIndex) {
  if (fileIndex >= 3) {
    printError("timeCompare mode is supported only two files");
  }

  outPutFile.get()->binHead.majorVer = BIN_MAJOR_VER_2;
  outPutFile.get()->binHead.minorVer = BIN_MINOR_VER_1;
  outPutFile.get()->majorVer = BIN_MAJOR_VER_2;
  outPutFile.get()->minorVer = BIN_MINOR_VER_1;
  uint32_t mergedFrame = 0;

  std::unique_ptr<BinFrameHead2d0[]> frameHeaderBuffer1 =
    std::make_unique<BinFrameHead2d0[]>(fileIndex);
  std::unique_ptr<BinFrameHead2d0[]> frameHeaderBuffer2 =
    std::make_unique<BinFrameHead2d0[]>(fileIndex);

  std::unique_ptr<char[]> dataBuffer1 =
    std::make_unique<char[]>(outPutFile.get()->binHead.singleFrameSz);
  std::unique_ptr<char[]> dataBuffer2 =
    std::make_unique<char[]>(outPutFile.get()->binHead.singleFrameSz);

  if (!outPutFile.get()->file.is_open()) {
    std::cerr << "Failed to open the file." << std::endl;
    return 1;
  }
  std::cout << "Current file pointer position: "
            << outPutFile.get()->file.tellp() << std::endl;

  /********************************************frameheader********************************************/
  for (uint32_t i = startFrmae; i < endFrmae; i++) {
    fileInfo.get()->file.seekg(fileInfo.get()->binHead.headerSz +
                               fileInfo.get()->binHead.singleFrameSz * i);

    // frameHeader의 timelog 읽어오기
    uint32_t min_time_diff = 100000000;
    uint64_t time = 0;
    uint32_t file2_min_frameNum = 0;
    uint32_t min_frameNum = 0;

    fileInfo[0].file.seekg(fileInfo[0].binHead.headerSz +
                           fileInfo[0].binHead.singleFrameSz * i);

    fileInfo[0].file.read((char*)frameHeaderBuffer1.get(),
                          sizeof(BinFrameHead2d0));

    for (uint32_t file2Frame = file2_min_frameNum;
         file2Frame < fileInfo[1].frameNum; file2Frame++) {
      fileInfo[1].file.seekg(fileInfo[1].binHead.headerSz +
                             fileInfo[1].binHead.singleFrameSz * file2Frame);
      fileInfo[1].file.read((char*)frameHeaderBuffer2.get(),
                            sizeof(BinFrameHead2d0));
      time = abs(static_cast<int64_t>(frameHeaderBuffer1.get()->timelog -
                                      frameHeaderBuffer2.get()->timelog));

      if (min_time_diff > time) {
        min_time_diff = time;
        file2_min_frameNum = file2Frame;
      }
    }

    if (min_time_diff > 200) {
      continue;
    } else {
      mergedFrame++;
      min_frameNum = file2_min_frameNum;

      outPutFile.get()->file.write(
        reinterpret_cast<char*>(frameHeaderBuffer1.get()),
        sizeof(BinFrameHead2d0));

      for (uint32_t j = 0; j < fileIndex; j++) {
        std::unique_ptr<TimeStamp> timeBuffer = std::make_unique<TimeStamp>();

        fileInfo[j].file.seekg(
          fileInfo[j].binHead.headerSz + fileInfo[j].binHead.singleFrameSz * i +
          sizeof(BinFrameHead2d0) + fileInfo[j].lidarNum * sizeof(TimeStamp));

        fileInfo[j].file.read((char*)timeBuffer.get(),
                              fileInfo[j].lidarNum * sizeof(TimeStamp));

        outPutFile.get()->file.write((char*)timeBuffer.get(),
                                     fileInfo[j].lidarNum * sizeof(TimeStamp));
      }

      fileInfo[0].file.seekg(
        fileInfo[0].binHead.headerSz + fileInfo[0].binHead.singleFrameSz * i +
        sizeof(BinFrameHead2d0) + fileInfo[0].lidarNum * sizeof(TimeStamp));

      fileInfo[0].file.read(dataBuffer1.get(),
                            fileInfo[0].binHead.singleFrameSz -
                              fileInfo[0].lidarNum * sizeof(TimeStamp) -
                              sizeof(BinFrameHead2d0));

      outPutFile.get()->file.write(dataBuffer1.get(),
                                   fileInfo[0].binHead.singleFrameSz -
                                     fileInfo[0].lidarNum * sizeof(TimeStamp) -
                                     sizeof(BinFrameHead2d0));

      fileInfo[1].file.seekg(fileInfo[1].binHead.headerSz +
                             fileInfo[1].binHead.singleFrameSz * min_frameNum +
                             sizeof(BinFrameHead2d0) +
                             fileInfo[1].lidarNum * sizeof(TimeStamp));

      fileInfo[1].file.read(dataBuffer2.get(),
                            fileInfo[1].binHead.singleFrameSz -
                              fileInfo[1].lidarNum * sizeof(TimeStamp) -
                              sizeof(BinFrameHead2d0));

      outPutFile.get()->file.write(dataBuffer2.get(),
                                   fileInfo[1].binHead.singleFrameSz -
                                     fileInfo[1].lidarNum * sizeof(TimeStamp) -
                                     sizeof(BinFrameHead2d0));
      outPutFile.get()->file.flush();
    }
  }
  outPutFile.get()->file.close();
  return 1;
}
