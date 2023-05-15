void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:
      ESP_LOGI(LOGTAG, "Wakeup caused by external signal using RTC_IO");
      break;
    case ESP_SLEEP_WAKEUP_EXT1:
      ESP_LOGI(LOGTAG, "Wakeup caused by external signal using RTC_CNTL");
      break;
    case ESP_SLEEP_WAKEUP_TIMER:
      ESP_LOGI(LOGTAG, "Wakeup caused by timer");
      break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
      ESP_LOGI(LOGTAG, "Wakeup caused by touchpad");
      break;
    case ESP_SLEEP_WAKEUP_ULP:
      ESP_LOGI(LOGTAG, "Wakeup caused by ULP program");
      break;
    default:
      ESP_LOGI(LOGTAG, "Wakeup was not caused by deep sleep: %d",
               wakeup_reason);
      break;
  }
}



void stateSet(int bit) {
  bitSet(state, bit);
  preferences.putBytes("state", &state, 1);
}
void stateClear(int bit) {
  bitClear(state, bit);
  preferences.putBytes("state", &state, 1);
}
bool stateRead(int bit) { return bitRead(state, bit); }