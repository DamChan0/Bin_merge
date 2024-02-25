#include "BinMerge.h"

#include <string.h>
#include <unistd.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

#include "fileControl.h"
#define UNUSED(x)  (void)(x)
#define ARR_NUM(x) (sizeof(x) / sizeof(*x))

#define HD_MARKER    "vueron"
#define FRAME_MARKER "frame"
// ANSI Escape codes for text colors
#define ANSI_COLOR_RED    "\033[91m"
#define ANSI_COLOR_RESET  "\033[0m"
#define ANSI_COLOR_PURPLE "\033[95m"
#define ANSI_COLOR_GREEN  "\033[92m"
#define ANSI_COLOR_WHITE  "\033[47m"
#define BOLD              "\033[1m"

#define LEGACY 0
uint32_t option = 0;

// Function to print a warning message in red color
void printWarning(const std::string& message) {
  std::cout << std::endl
            << ANSI_COLOR_PURPLE << "Warning!!!: " << message
            << ANSI_COLOR_RESET << std::endl;
  std::cout << "\033[0m";
}

void printTime(uint32_t sec, uint32_t min) {
  std::cout << ANSI_COLOR_PURPLE ANSI_COLOR_WHITE BOLD << "PLAYTIME: " << min
            << "분" << sec << "초" << ANSI_COLOR_RESET << std::endl;
  std::cout << "\033[0m";
}

void printError(const std::string& message) {
  std::cout << std::endl
            << ANSI_COLOR_RED << "ERROR!!!: " << message << ANSI_COLOR_RESET
            << std::endl;
  std::cout << "\033[0m";
  std::exit(1);  // 에러 코드 1로 종료
}

void printCorrect(const std::string& message) {
  std::cout << std::endl
            << ANSI_COLOR_GREEN << "SUCCESS!: " << message << ANSI_COLOR_RESET
            << std::endl;
  std::cout << "\033[0m";
}

void printDebug(const std::string& message, const std::string& flag) {
  std::cout << ANSI_COLOR_GREEN ANSI_COLOR_WHITE BOLD << flag << message
            << ANSI_COLOR_RESET << std::endl;
  std::cout << "\033[0m";
}

int BinMergeMain::Merge::BinMerge(int argc, std::vector<std::string> argv,
                                  uint32_t option, uint32_t startFrame,
                                  uint32_t endFrame) {
  MergeLidarData::updata_File input_files;
  MergeLidarData::updata_File merged_file;
  uint32_t fileIndex =
    input_files.checkOpt(argc, argv, &option, startFrame, endFrame);
  input_files.checkHeaderInfo(fileIndex);
  if (input_files.read_File_Header(fileIndex)) {
    printCorrect("compliete read headerinfo, sensorinfo, calibrationinfo");
  } else {
    printError("error read header");
  };
  for (int i = 0; i < 2; i++) {
    for (uint32_t j = 0; j < input_files.fileInfo[i].lidarNum; j++) {
      printf("subid :%s\n", input_files.fileInfo[i].sensors[j].subId);
    }
  }
  input_files.merge_Header(fileIndex);

  for (uint32_t j = 0; j < input_files.outPutFile.get()->lidarNum; j++) {
    printf("outputfile subid :%s\n",
           input_files.outPutFile.get()->sensors[j].subId);
  }

  switch (input_files.mergeOption) {
    case 1:
      if (startFrame == 0 && endFrame == 0) {
        input_files.merge_Frame(0, input_files.outPutFile.get()->frameNum,
                                fileIndex);
      } else if (startFrame == 0 && endFrame != 0) {
        input_files.merge_Frame(0, endFrame, fileIndex);
      } else if (startFrame != 0 && endFrame != 0) {
        input_files.merge_Frame(startFrame, endFrame, fileIndex);
      }
      break;
    case 2:
      if (startFrame == 0 && endFrame == 0) {
        input_files.merge_Frame_Timecompare(
          0, input_files.outPutFile.get()->frameNum, fileIndex);
      } else if (startFrame == 0 && endFrame != 0) {
        input_files.merge_Frame_Timecompare(0, endFrame, fileIndex);
      } else if (startFrame != 0 && endFrame != 0) {
        input_files.merge_Frame_Timecompare(startFrame, endFrame, fileIndex);
      }
      break;
    case 3:
      if (fileIndex > 1) {
        printError("Too Many input files");
      }
      input_files.split(input_files.splitNum_start, input_files.splitNum_end);
      break;

    default:
      break;
  }
  return 0;
}