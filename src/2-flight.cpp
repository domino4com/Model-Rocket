#include "2-flight.h"

#include <SPIFFS.h>
#include <SPL06-007.h>
#include <Wire.h>
#include <kxtj3-1057.h>
// Samples per second in Hz:
// 0.781, 1.563, 3.125, 6.25, 12.5, 25, 50, 100, 200, 400, 800, 1600
float IIAsampleRate = 1600;
uint8_t IIAaccelRange = 16;  // Accelerometer range = 2, 4, 8, 16g
#define HIGH_RESOLUTION
KXTJ3 IIA(0x0F);

File data;
Record record = {0, 0, 0, 0, 0, 0};
byte* pointer = (byte*)&record;
size_t sizeRecord = sizeof(record);
long counter = 0;

bool Flight::setup_flight() {
  Wire.begin();
  Wire.setClock(3400000);

  if (IIA.begin(IIAsampleRate, IIAaccelRange) != 0) {
    ESP_LOGE(LOGTAG, "Failed to initialize IIA:Accelerometer");
    return false;
  } else {
    ESP_LOGI(LOGTAG, "Initialize IIA:Accelerometer succesfully");
  }

  if (IIA.axisAccel(X) < 1) {
    ESP_LOGI(LOGTAG,
             "Waiting for upright at launchpad. Sleeping for 5 seconds...");
    IIA.standby(true);
    esp_sleep_enable_timer_wakeup(5000000);  // 5 seconds
    Serial.flush();
    esp_deep_sleep_start();
  }
  ESP_LOGI(LOGTAG, "Rocket is Upright, a bit more preparation is needed.");
  if (!SPIFFS.begin(true)) {
    ESP_LOGE(LOGTAG, "An Error has occurred while mounting SPIFFS");
    return false;
  }
  data = SPIFFS.open(FILEPATH, FILE_WRITE);
  if (!data) {
    ESP_LOGE(LOGTAG, "Failed to open data file for writing");
    SPIFFS.end();
    return false;
  }
  ESP_LOGI(LOGTAG, "Bytes: Total %zu - Used %zu", SPIFFS.totalBytes(),
           SPIFFS.usedBytes());
  SPL_init(0x77);
  IIA.standby(false);
  return true;
}

bool Flight::flight() {
  ESP_LOGI(LOGTAG, "Now waiting for takeoff");
  // TODO
  unsigned long start = millis();
  while (true) {
    record.time = millis();
    record.accelX = IIA.axisAccel(X);
    record.accelY = IIA.axisAccel(Y);
    record.accelZ = IIA.axisAccel(Z);
    if (counter % 10 == 0) {  // every 10th
      record.temp = get_temp_c();
      record.pressure = get_pressure();
    }
    if (!data.write(pointer, sizeRecord))
      break;  // Storage full, so no more to do here!
    if (counter++ % 1000 == 0) data.flush();  // Save every 1000
  }
  unsigned long end = millis();
  data.close();
  IIA.standby(true);
  ESP_LOGI(LOGTAG, "Bytes: Total %zu - Used %zu", SPIFFS.totalBytes(),
           SPIFFS.usedBytes());
  ESP_LOGI(LOGTAG, "Flight time: %lu - per transaction: %f", end - start,
           ((end - start) / (float)counter));

  SPIFFS.end();
  Wire.end();
  return true;
}
