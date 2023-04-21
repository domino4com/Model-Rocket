#include "1-prepare.h"
#include <math.h>

bool Prepare::prepare() {
  ESP_LOGI(LOGTAG, "Preparation: Formatting storage - will take 34-35 seconds!");
  unsigned long start = millis();

  if (!SPIFFS.begin(true)) {
    ESP_LOGE(LOGTAG, "An Error has occurred while mounting SPIFFS");
    return false;
  }
  if (!SPIFFS.format()) {
    ESP_LOGE(LOGTAG, "An Error has occurred while formatting SPIFFS");
    SPIFFS.end();
    return false;
  }

  ESP_LOGI(LOGTAG, "Storage formatted - time: %d s", (int)(round((float)((millis() - start)/1000.0f))));
  ESP_LOGI(LOGTAG, "Total: %d KB - Used: %ld B", (int)(round((float)(SPIFFS.totalBytes()/1024.0f))),
           SPIFFS.usedBytes());
  SPIFFS.end();
  ESP_LOGI(LOGTAG, "Preparation completed");
  return true;
}