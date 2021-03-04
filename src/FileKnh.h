#ifndef FILEKNH_H
#define FILEKNH_H

// Libraries for SD card
#include "FS.h"
//#include "SD_MMC.h" // cam
#include "SD.h" // non-cam
#include <SPI.h>
#include "Buff.h"

class FileKnh {
    public:
    FileKnh();
    void initSdFs();

    void appendFile(
        fs::FS &fs, 
        const char* path, 
        const char* message);
        
    void writeFile(
        fs::FS &fs, 
        const char* path,
        const char* message);

    void appendBuffToFile(
        fs::FS &fs, 
        const char* path,
        Buff &buff);

    void deleteFile(
        fs::FS &fs, 
        const char* path);

    void testAppendBuffToFile(
        fs::FS &fs, 
        const char* path,
        Buff &buff,
        int recordCount);
};

#endif