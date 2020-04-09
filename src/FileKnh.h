#ifndef FILEKNH_H
#define FILEKNH_H

// Libraries for SD card
#include "FS.h"
#include "SD.h"
#include <SPI.h>
#include "Buff.h"

class FileKnh {
    public:
    FileKnh();
    int sdCsPin;
    void initSdFs(int sdCsPinIn);

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