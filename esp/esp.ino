#include <FS.h>
#include <LittleFS.h>

#define ARDUINO_RESET D1

File file;

#define VIDEO_PERIOD (1000000 / 30)
#define VIDEO_FRAME_SIZE 175

void setup() {
  Serial.begin(115200);
  Serial.println("hi");

  if(!LittleFS.begin()) {
    Serial.println("mount fail");
    return;
  }

  file = LittleFS.open("/bad_apple.bin", "r");
  if(file) {
    Serial.println("file open OK");
  } else {
    Serial.println("file open fail");
    return;
  }
  
  Serial1.begin(115200);
  pinMode(ARDUINO_RESET, OUTPUT_OPEN_DRAIN);
  digitalWrite(ARDUINO_RESET, LOW);
  delay(100);
  digitalWrite(ARDUINO_RESET, HIGH);
  delay(3000);
}

void loop() {
  static size_t video_pos = 0;

  int start_us = micros();
  for(size_t i = 0; i < VIDEO_FRAME_SIZE; i++) {
    Serial1.write(file.read());
  }
  int took_us = micros() - start_us;
  int left_us = VIDEO_PERIOD - took_us;
  if(left_us) delayMicroseconds(left_us);

  if(!file.available()) {
    ESP.restart();
  }
}
