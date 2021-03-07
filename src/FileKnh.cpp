#include "FileKnh.h"

FileKnh::FileKnh() {
}

// cam
// void FileKnh::initSdFs() {
//   // Initialize SD card
//   SD_MMC.begin();  
//   if(!SD_MMC.begin()) {
//     Serial.println("Card Mount Failed");
//     return;
//   }

//   uint8_t cardType = SD_MMC.cardType();
//   if(cardType == CARD_NONE) {
//     Serial.println("No SD card attached");
//     return;
//   }

//   Serial.println("Initializing SD card...");
//   if (!SD_MMC.begin()) {
//     Serial.println("ERROR - SD card initialization failed!");
//     return;    // init failed
//   }

//   // // If the data.txt file doesn't exist
//   // // Create a file on the SD card and write the data labels
//   // File file = SD.open("/data.txt");
//   // if(!file) {
//   //   Serial.println("File doens't exist");
//   //   Serial.println("Creating file...");
//   //   writeFile(SD, "/data.txt", "Reading ID, Date, Hour, Temperature \r\n");
//   // }
//   // else {
//   //   Serial.println("File already exists");  
//   // }
//   // file.close();
// }


// non-cam
void FileKnh::initSdFs() {
  // Initialize SD card
  int sdcs = 5;
  SD.begin(sdcs);
  if(!SD.begin(sdcs)) {
    Serial.println("Card Mount Failed");
    return;
  }

  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.println("Initializing SD card...");
  if (!SD.begin(sdcs)) {
    Serial.println("ERROR - SD card initialization failed!");
    return;    // init failed
  }
}

void FileKnh::writeFile(
    fs::FS &fs, 
    const char * path,
    const char * message) {
  
  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  
  if(file.print(message)) {
    Serial.println("File written");
  } 
  else {
    Serial.println("Write failed");
  }
  file.close();
}

void FileKnh::appendFile(
    fs::FS &fs, 
    const char * path, 
    const char * message) {

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  
  if(file.print(message)) {
    Serial.println("Message appended");
  } 
  else {
    Serial.println("Append failed");
  }
  file.close();
}

void FileKnh::appendBuffToFile(
    fs::FS &fs, 
    const char * path,
    Buff &buff) {

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Failed to open file for appending");
    return;
  }  

  Sample sample;
  bool isMore = true;
  // empty the one full buffer 
  // (we keep two buffers so that we can empty one while writing to the other)
  while (isMore) {
    sample = buff.GetNext(isMore);    
    String v = SampleToString(sample);    
    file.print(v);
    // if(file.print(v)) {
    //   Serial.println("Message appended");
    // } 
    // else {
    //   Serial.println("Append failed");
    // }
  }

  file.close();
}

void FileKnh::deleteFile(
    fs::FS &fs, 
    const char* path) {
      fs.remove(path);
}

void FileKnh::testAppendBuffToFile(
    fs::FS &fs, 
    const char* path,
    Buff &buff,
    int recordCount) {

  int startMs = millis();
  
  for(int i = 0; i < recordCount; i++) {
    Sample sample;
    sample.mills = i + 1;
    sample.sample = -1 * (i + 1);
    bool isFull = false;
    buff.Push(sample, isFull);
    
    if (isFull) {
      appendBuffToFile(
        fs, 
        path,
        buff);
    }
  }
  int endMs = millis();

  Serial.println("testAppendBuffToFile, count: " 
    + String(recordCount)
    + " MS: " 
    + String(endMs - startMs));
}