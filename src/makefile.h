#ifndef BIN_DATA_INFO_H1
#define BIN_DATA_INFO_H1
#include <string.h>
#include <type.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

#include "BinFormat.h"
#include "BinLegacyFormat.h"

#define BUILD_DIR "build/"
void _write_frameL00(BinDataInfo_t* input_F, BinDataInfo_t* input_F2,
                     uint8_t* framedata1, uint8_t* framedata2);
void make_fileL00(BinDataInfo_t* file1, BinDataInfo_t* file2,
                  BinDataInfo_t* mergefile);
void make_fileL11(LegacyBinFileInfo_t* file1, LegacyBinFileInfo_t* file2,
                  BinDataInfo_t* mergefile);
void make_fileL01(BinDataInfo_t* file1, LegacyBinFileInfo_t* file2,
                  BinDataInfo_t* mergefile);
void make_fileL10(LegacyBinFileInfo_t* file1, BinDataInfo_t* file2,
                  BinDataInfo_t* mergefile);

void write_headerL00(BinDataInfo_t* file1, BinDataInfo_t* file2,
                     BinDataInfo_t* temp);
void write_headerL11(LegacyBinFileInfo_t* file1, LegacyBinFileInfo_t* file2,
                     BinDataInfo_t* convertBinFileInf1,
                     BinDataInfo_t* convertBinFileInf2,
                     BinDataInfo_t* mergefile);
void write_headerL01(BinDataInfo_t* file1, LegacyBinFileInfo_t* file2,
                     BinDataInfo_t* convertBinFileInf1,
                     BinDataInfo_t* mergefile);
void write_headerL10(LegacyBinFileInfo_t* file1, BinDataInfo_t* file2,
                     BinDataInfo_t* convertBinFileInf1,
                     BinDataInfo_t* mergefile);

void write_frameL00(BinDataInfo_t* file1, BinDataInfo_t* file2,
                    BinDataInfo_t* mergefile, uint8_t* data1[],
                    uint8_t* data2[]);
void write_frameL11(LegacyBinFileInfo_t* file1, LegacyBinFileInfo_t* file2,
                    BinDataInfo_t* convertBinFileInf1,
                    BinDataInfo_t* convertBinFileInf2,
                    BinDataInfo_t* mergefile);
void write_frameL01(BinDataInfo_t* file1, LegacyBinFileInfo_t* file2,
                    BinDataInfo_t* convertBinFileInf1,
                    BinDataInfo_t* mergefile);
void write_frameL10(LegacyBinFileInfo_t* file1, BinDataInfo_t* file2,
                    BinDataInfo_t* convertBinFileInf1,
                    BinDataInfo_t* mergefile);

void merge_BinFile_L00(BinDataInfo_t* temp_File1, BinDataInfo_t* temp_File2,
                       BinDataInfo_t* mergefile, uint8_t* data1[],
                       uint8_t* data2[]);
void merge_BinFile_L11(LegacyBinFileInfo_t* temp_File1,
                       LegacyBinFileInfo_t* temp_File2,
                       BinDataInfo_t* convertBinFileInf1,
                       BinDataInfo_t* convertBinFileInf2,
                       BinDataInfo_t* mergefile);
void merge_BinFile_L10(LegacyBinFileInfo_t* temp_File1,
                       BinDataInfo_t* temp_File2,
                       BinDataInfo_t* convertBinFileInf1,
                       BinDataInfo_t* mergefile);
void merge_BinFile_L01(BinDataInfo_t* temp_File1,
                       LegacyBinFileInfo_t* temp_File2,
                       BinDataInfo_t* convertBinFileInf1,
                       BinDataInfo_t* mergefile);
#endif