// #include <stddef.h>
// #include <unistd.h>

// #include <cstdint>
// #include <cstdlib>
// #include <cstring>
// #include <fstream>
// #include <iostream>
// #include <memory>
// #include <sstream>
// #include <string>

// #include "writeLegacybin.h"

// void binHeadInitFromLegacyBin(BinFileInfo_t* dst, LegacyBinFileInfo_t* src) {
//   strcpy(dst->binHead.marker, "vueron");
//   dst->binHead.majorVer = 1;
//   dst->binHead.minorVer = 1;
//   dst->binHead.singleFrameSz = 0;
//   dst->binHead.sensorCnt = src->sens.cnt;
//   dst->binHead.calCnt = src->sens.cnt;
//   dst->binHead.headerSz = sizeof(BinHeadInfo_t) +
//                           (sizeof(BinHeadSensor_t) * dst->binHead.sensorCnt) +
//                           (sizeof(BinHeadCal_t) * dst->binHead.calCnt);
//   for (size_t i = 0; i < dst->binHead.sensorCnt; i++) {
//     dst->binHead.singleFrameSz += (src->sens.maxBufSz[i]);
//   }
//   dst->binHead.singleFrameSz += sizeof(BinFrameHead_t);

//   sprintf(dst->binHead.str,
//           "ver: %d.%d,headerSz: %d,sensorCnt:%d,singleFrmSz:%d",
//           dst->binHead.majorVer, dst->binHead.minorVer, dst->binHead.headerSz,
//           dst->binHead.sensorCnt, dst->binHead.singleFrameSz);
//   cout << dst->binHead.str << endl;

//   // create new bin file name
//   string tmp = src->fileName;
//   tmp.insert(tmp.size() - 4, "_new");
//   std::strcpy(dst->fileName, tmp.c_str());

//   // write here
//   dst->ofile.open(dst->fileName);
//   dst->ofile.write((char*)&dst->binHead, sizeof(dst->binHead));
// }

// void binSenserHeadInitFromLegacyBin(BinFileInfo_t* dst,
//                                     LegacyBinFileInfo_t* src) {
//   for (size_t i = 0; i < src->sens.cnt; i++) {
//     dst->sensors[i].frameSz = src->sens.maxBufSz[i];
//     dst->sensors[i].sensorId = (uint32_t)src->sens.id[i];
//     sprintf(dst->sensors[i].subId, "lidar_%ld", i);
//     sprintf(dst->sensors[i].str, "frmSz:%d, sensorID:%d",
//             dst->sensors[i].frameSz, dst->sensors[i].sensorId);
//   }
//   dst->ofile.write((char*)dst->sensors, sizeof(*dst->sensors) * src->sens.cnt);
// }

// void binSetCalInitFromLegacyBin(BinFileInfo_t* dst, LegacyBinFileInfo_t* src) {
//   for (size_t i = 0; i < src->sens.cnt; i++) {
//     memset(&dst->cal[i], 0, sizeof(*dst->cal));
//     sprintf(dst->cal[i].str, "cal-id: #%d", dst->sensors[i].sensorId);
//   }
//   // write here
//   dst->ofile.write((char*)dst->cal, sizeof(*dst->cal) * src->sens.cnt);
// }

// void binWriteBodyFromLegacyBin(BinFileInfo_t* dst, LegacyBinFileInfo_t* src) {
//   LegacyBinFrameStartInfo_t lgcFrmHd;
//   LegacyBinFrameMidInfo_t lgcFrmMid;
//   BinFrameHead_t frmHd;
//   char* data[dst->binHead.sensorCnt];
//   memset(&frmHd, 0, sizeof(frmHd));

//   // re-open file
//   src->file->close();
//   src->file->open(src->fileName);

//   // memset(data, 0, sizeof(*data) * dst->binHead.sensorCnt);
//   for (size_t i = 0; i < dst->binHead.sensorCnt; i++) {
//     data[i] = NULL;
//     data[i] = (char*)malloc(dst->sensors[i].frameSz);
//   }

//   for (size_t i = 0; i < src->frameNum; i++) {
//     uint64_t ofs = src->frameIdx[i];
//     // read frame header
//     src->file->seekg(ofs);
//     src->file->read((char*)&lgcFrmHd, sizeof(lgcFrmHd));
//     if (strcmp(lgcFrmHd.delim, "FrameNumber")) {
//       printf("Error. %s", lgcFrmHd.delim);
//       exit(1);
//     }

//     // write frame header
//     frmHd.frameNum = i;
//     strcpy(frmHd.frameMarker, (char*)"frame");
//     sprintf(frmHd.str, "#%d", (int)i);
//     dst->ofile.write((char*)&frmHd, sizeof(frmHd));
//     dst->ofile.flush();
//     for (size_t j = 0; j < src->sens.cnt; j++) {
//       // read data
//       memset(data[j], 0, src->sens.maxBufSz[j]);
//       src->file->read((char*)&lgcFrmMid, sizeof(lgcFrmMid));
//       src->file->read(data[j], src->sens.maxBufSz[j]);
//       // write data
//       dst->ofile.write(data[j], src->sens.maxBufSz[j]);
//       dst->ofile.flush();
//     }
//     cout << '#';
//     if (i % 80 == 0) {
//       cout << endl;
//     }
//     fflush(stdout);
//   }

//   for (size_t i = 0; i < dst->binHead.sensorCnt; i++) {
//     free(data[i]);
//   }
// }