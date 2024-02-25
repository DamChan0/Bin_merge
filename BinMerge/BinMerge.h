#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
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

#define SENSOR_NAME_LEN 32

typedef float float32_t;

using namespace std;

#define CUR_BIN_MAJOR  BIN_MAJOR_VER_2
#define CUR_BIN_MINOR  BIN_MINOR_VER_NONE
#define NOT_LEGACY_BIN 0

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
  BIN_MINOR_VER_NONE,
  BIN_MINOR_VER_1,
  BIN_MINOR_VER_2,
  BIN_MINOR_VER_MAX,
} eBinMinorVersion;

typedef enum {
  BIN_MAJOR_VER_NONE,
  BIN_MAJOR_VER_1,
  BIN_MAJOR_VER_2,
  BIN_MAJOR_VER_MAX,
} eBinMajorVersion;

namespace BinMergeMain {

class Merge {
 public:
  int BinMerge(int argc, std::vector<std::string> argv, uint32_t option,
               uint32_t splitStart, uint32_t splitEnd);
};
}  // namespace BinMergeMain
void printWarning(const std::string& message);
void printTime(uint32_t sec, uint32_t min);
void printError(const std::string& message);
void printCorrect(const std::string& message);
void printDebug(const std::string& message, const std::string& flag);

#endif  // __MAIN_H__