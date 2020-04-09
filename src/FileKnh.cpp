#include "FileKnh.h"

FileKnh::FileKnh() {
    sdCsPin = 0;
}


void FileKnh::initSdFs(int sdCsPinIn) {
    sdCsPin = sdCsPinIn;

  // Initialize SD card
  SD.begin(sdCsPin);  
  if(!SD.begin(sdCsPin)) {
    Serial.println("Card Mount Failed");
    return;
  }

  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.println("Initializing SD card...");
  if (!SD.begin(sdCsPin)) {
    Serial.println("ERROR - SD card initialization failed!");
    return;    // init failed
  }

  // // If the data.txt file doesn't exist
  // // Create a file on the SD card and write the data labels
  // File file = SD.open("/data.txt");
  // if(!file) {
  //   Serial.println("File doens't exist");
  //   Serial.println("Creating file...");
  //   writeFile(SD, "/data.txt", "Reading ID, Date, Hour, Temperature \r\n");
  // }
  // else {
  //   Serial.println("File already exists");  
  // }
  // file.close();
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
  while (isMore) {
    sample = buff.GetNext(isMore);
    
    String v = String(sample.mills) + "\r\n";
    //Serial.println(v);
    
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
    sample.mills = i;
    sample.sample = -1 * i;
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