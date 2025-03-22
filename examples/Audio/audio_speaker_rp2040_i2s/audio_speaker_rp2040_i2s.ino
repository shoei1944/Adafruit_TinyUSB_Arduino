/*********************************************************************
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 Copyright (c) 2024 Phl Schatzmann

 This example uses a Rasperry Pico to send the output to i2s. To prevent
 any underruns or overruns we take care of the feedback ourself. 
 
*********************************************************************/
#include "Adafruit_TinyUSB.h"
#include "AudioTools.h"
#include "AudioTools/Concurrency/RP2040.h"

AudioInfo info(44100, 2, 16);
Adafruit_USBD_Audio usb;
BufferRP2040 buffer(256, 20); 
QueueStream queue(buffer);
I2SStream i2s;
StreamCopy copier(i2s, queue);

size_t writeCB(const uint8_t* data, size_t len, Adafruit_USBD_Audio& ref) {
  usb.setFeedbackPercent(buffer.size()*100 / buffer.available());
  return queue.write(data, len);
}

void setup() {  
  Serial.begin(115200);
  AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Warning);
  //while(!Serial);  // wait for serial
  delay(1000);
  Serial.println("starting...");
  // start queue
  queue.begin(80);

  // Start USB device as Audio Sink w/o automatic feedback
  usb.setFeedbackMethod(AUDIO_FEEDBACK_METHOD_DISABLED);
  usb.setWriteCallback(writeCB);
  if (!usb.begin(info.sample_rate, info.channels, info.bits_per_sample)){
    Serial.println("USB error");
  }

  // If already enumerated, additional class driverr begin() e.g msc, hid, midi won't take effect until re-enumeration
  if (TinyUSBDevice.mounted()) {
    TinyUSBDevice.detach();
    delay(10);
    TinyUSBDevice.attach();
  }
}

void loop() {
  usb.updateLED();
}

void setup1(){
  //start i2s
  auto cfg = i2s.defaultConfig(TX_MODE);
  cfg.copyFrom(info);
  cfg.buffer_size = 256;
  cfg.buffer_count = 3;
  if (!i2s.begin(cfg)){
    Serial.print("i2s error");
  }
}

void loop1() {
  copier.copy();
}