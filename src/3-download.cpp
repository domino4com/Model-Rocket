#include "3-download.h"
#include "XModem.h"

bool Download::download(Stream *port) {
  ESP_LOGI(LOGTAG, "Download getting ready");
  if (!SPIFFS.begin(false)) {
    ESP_LOGE(LOGTAG, "An Error has occurred while mounting SPIFFS");
    return false;
  }
  File downloadData = SPIFFS.open(FILEPATH, FILE_READ);
  if (downloadData) {
    Serial.println("Start the YMODEM download on your computer!");
    XModem xmodem(port, ModeYModem);
    xmodem.sendFile(downloadData, FILENAME);
    downloadData.close();
  } else {
    ESP_LOGE(LOGTAG,"An Error has occurred while opening datalog.txt");
    return false;
  }
  SPIFFS.end();
  return true;
}