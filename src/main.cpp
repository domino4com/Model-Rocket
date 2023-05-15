#include "1-prepare.h"
Prepare prepare;
#include "2-flight.h"
Flight flight;
#include "3-download.h"
Download download;

#include <Preferences.h>

#include <bitset>

#include "CapTouch.h"
#include "LED.h"
#include "global.h"
Preferences preferences;
byte state;
#include "utilities.h"
#include "remote.h"


void setup() {
  Serial.begin(115200);
  delay(1000);
  RemoteXY_Init();
  orig_esp_log = esp_log_set_vprintf(ble_log);
  ESP_LOGI(LOGTAG, "Rocket Firmware V1.0");
  print_wakeup_reason();
  CapTouchSetup();
  LEDsetup();
  action(false);
  preferences.begin(LOGTAG, false);
  if (preferences.isKey("state")) {
    preferences.getBytes("state", &state, 1);
  } else {
    state = 0;
  }
  ESP_LOGI(LOGTAG, "State: %s",
           String(std::bitset<8>(state).to_string().c_str()));
  if (stateRead(Prepared)) {
    ESP_LOGI(LOGTAG, "Rocket prepared");
  }
  if (stateRead(Armed)) {
    ESP_LOGI(LOGTAG, "Rocket armed");
  }
  if (stateRead(Upright)) {
    ESP_LOGI(LOGTAG, "Rocket upright");
  }
  if (stateRead(Completed)) {
    ESP_LOGI(LOGTAG, "Rocket completed");
  }
}

void loop() {
  RemoteXY_Handler();
  log_state();

  // Preparing Step 1
  if (buttons(2, left, right) || (RemoteXY.activity == 0 && RemoteXY.execute)) {
    action(true);
    state = (byte)prepare.prepare();
    preferences.putBytes("state", &state, 1);
    action(false);
  }

  // Flight Step 2
  if (buttons(1, check) || (RemoteXY.activity == 1 && RemoteXY.execute) ||
      stateRead(Armed)) {
    if (!stateRead(Prepared)) {
      ESP_LOGE(LOGTAG, "Rocket not prepared - abort launch sequence");
      stateClear(Armed);  // Just in case
    } else {
      action(true);
      stateSet(Armed);
      if (flight.setup_flight()) {
        ESP_LOGI(LOGTAG, "Rocket upright at launchpad");
        stateClear(Armed);
        stateSet(Upright);
        tach(false);
        rec(true);
        flight.flight();
        rec(false);
        tach(true);
        stateClear(Prepared);
        stateSet(Completed);
        ESP_LOGI(LOGTAG, "Flight completed");
      } else {
        ESP_LOGE(LOGTAG, "Flight setup failed");
        stateClear(Armed);
      }
      action(false);
    }
  }

  // Downloading Step 3
  if (buttons(1, up) || (RemoteXY.activity == 2 && RemoteXY.execute)) {
    if (!stateRead(Completed)) {
      ESP_LOGE(LOGTAG, "Flight not completed, no data - abort launch sequence");
    } else {
      action(true);
      while (!download.download(&Serial)) {
        ESP_LOGW(LOGTAG, "Download failed, retrying");
      }
      ESP_LOGI(LOGTAG, "Download completed!");
      action(false);
    }
  }

  // Dump
  if (buttons(1, cross)) {
    ESP_LOGI(LOGTAG, "Data dump");
    if (!SPIFFS.begin(false)) {
      ESP_LOGE(LOGTAG, "An Error has occurred while mounting SPIFFS");
    } else {
      ESP_LOGI(LOGTAG, "Bytes Total: %ld - Used: %ld", SPIFFS.totalBytes(),
               SPIFFS.usedBytes());
      File downloadData = SPIFFS.open(FILEPATH, FILE_READ);
      if (downloadData) {
        Record record;
        size_t sizePayload = sizeof(record);
        byte bufferInfo[sizePayload];
        while (downloadData.available()) {
          uint32_t nBytes =
              downloadData.readBytes((char*)bufferInfo, sizePayload);
          memcpy(&record, bufferInfo, sizePayload);
          ESP_LOGI(LOGTAG, "Time: %lu, X: %f, Y: %f, Z: %f, Temp: %f, Pres: %f",
                   record.time, record.accelX, record.accelY, record.accelZ,
                   record.temp, record.pressure);
        }
        downloadData.close();
      }
      SPIFFS.end();
    }
  }
  RemoteXY.execute = 0;
  button = 0;
}
