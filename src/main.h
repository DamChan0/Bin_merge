#include <string.h>
#include <type.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

#include "BinFormat.h"
#include "BinLegacyFormat.h"
#include "makefile.h"
#include "writeLegacybin.h"

void merge_BinFile(BinDataInfo_t* file1, BinDataInfo_t* file2);
void read_BinFile(BinDataInfo_t file1, BinDataInfo_t file2);  // file 정보 읽기
void read_LegBinFile(LegacyBinFileInfo_t file1,
                     LegacyBinFileInfo_t file2);  // file 정보 읽기
void make_file();