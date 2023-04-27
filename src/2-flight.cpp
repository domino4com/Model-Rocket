#include "2-flight.h"

#include <SPIFFS.h>
#include <SPL06-007.h>
#include <Wire.h>
#include <kxtj3-1057.h>

// TODO VVVVV allow to change this in the app, store in pref and reboot
//  Samples per second in Hz:
//  0.781, 1.563, 3.125, 6.25, 12.5, 25, 50, 100, 200, 400, 800, 1600
float IIAsampleRate = 1600;
uint8_t IIAaccelRange = 8;  // Accelerometer range = 2, 4, 8, 16g
#define HIGH_RESOLUTION
KXTJ3 IIA(0x0F);
// TODO ^^^^^^^ allow to change this in the app, store in pref and reboot

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
float calculateAltitude(float temperature, float pressure) {
  const double R = 8.31432;    // gas constant
  const double g0 = 9.80665;   // standard gravity at sea level
  const double M = 0.0289644;  // molar mass of air
  const double T0 = 288.15;    // temperature at sea level
  const double p0 = 101325.0;  // pressure at sea level
  const double L = 0.0065;     // temperature lapse rate
  const double h0 = 0.0;       // altitude at sea level

  double T = temperature;
  double p = pressure;

  // Calculate temperature and pressure at altitude
  double T1 = T0 - L * h0;
  double p1 = p0 * std::pow((T1 / T0), (g0 * M) / (R * L));

  double h =
      h0 + ((R * T1) / (g0 * M)) * (std::pow((p / p1), (1.0 / 5.257)) - 1.0);

  return h;
}
void analayze_flight() {
  data = SPIFFS.open(FILEPATH, FILE_READ);
  if (!data) {
    ESP_LOGE(LOGTAG, "Failed to open data file for reading");
    return;
  }

  float maxMagnitude = 0, maxAltitude = 0, minPressure = 0,
        matchTemperature = 0;
  Record record;
  size_t sizePayload = sizeof(record);
  byte bufferInfo[sizePayload];
  while (data.available()) {
    uint32_t nBytes = data.readBytes((char*)bufferInfo, sizePayload);
    if (nBytes != sizePayload) {
      ESP_LOGE(LOGTAG, "Failed to read data file");
      break;
    }
    memcpy(&record, bufferInfo, sizePayload);
    float magnitude =
        sqrt(record.accelX * record.accelX + record.accelY * record.accelY +
             record.accelZ * record.accelZ);
    if (magnitude > maxMagnitude) maxMagnitude = magnitude;
    if (record.pressure < minPressure) {
      minPressure = record.pressure;
      matchTemperature = record.temp;
    }
  }
  ESP_LOGI(LOGTAG, "Max acceleration: %f", maxMagnitude);
  maxAltitude = calculateAltitude(matchTemperature, minPressure);
  ESP_LOGI(LOGTAG, "Max altitude: %f", maxAltitude);
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
  analayze_flight();
  SPIFFS.end();
  Wire.end();
  return true;
}
