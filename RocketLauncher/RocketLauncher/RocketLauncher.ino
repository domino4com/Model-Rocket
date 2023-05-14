//OCA
#include "Arduino.h"
#include "PCF8574.h"

PCF8574 OCA(0x20);
#define MOSFET1 P0
#define MOSFET2 P1
#define GPIO3 P2
#define GPIO4 P3
//OCA

#include <WiFi.h>
#include <WebServer.h>
// #define WL_top_red 25
// #define WL_top_blue 26
// #define WL_mid_yellow 19  // Actually Green
// #define WL_bot_blue 18    // Actually Olive
// #define WL_bot_red 17
/* Put your SSID & Password */
const char* ssid = "RocketLauncher";  // Enter SSID here
const char* password = "toothpick";   //Enter Password here

/* Put IP Address details */
IPAddress local_ip(11, 11, 11, 11);
IPAddress gateway(11, 11, 11, 11);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);

// uint8_t LED1pin = WL_top_blue;
// bool LED1status = LOW;

// uint8_t LED2pin = WL_mid_yellow;
bool launchStatus = LOW;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("Rocket Launcher");
  //OCA
  Wire.begin();
  OCA.pinMode(MOSFET1, OUTPUT);
  OCA.pinMode(MOSFET2, OUTPUT);
  OCA.pinMode(GPIO3, OUTPUT);
  OCA.pinMode(GPIO4, OUTPUT);
  if (!OCA.begin()) {
    Serial.println("OCA not detected. Please check wiring. Freezing...");
    vTaskDelete(NULL);
  }
  OCA.digitalWrite(MOSFET1, LOW);
  OCA.digitalWrite(MOSFET2, LOW);
  OCA.digitalWrite(GPIO3, LOW);
  OCA.digitalWrite(GPIO4, LOW);
  //OCA
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);

  server.on("/", handle_OnConnect);
  server.on("/launch", handle_launch);
  server.on("/abort", handle_abort);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  int a = WiFi.getTxPower();
  Serial.print("TX power:");
  Serial.println(a);
}
long launchTimer = 0;
int currentSec = 0;
void loop() {
  server.handleClient();

  if (launchStatus) {
    int nowSec = (millis() - launchTimer) / 1000;
    if (nowSec != currentSec) {
      currentSec = nowSec;
      Serial.printf("T-%d\n", 11 - currentSec);
      server.send(200, "text/html", SendHTML(launchStatus, 11 - currentSec));
    }
    if (millis() - launchTimer > 11000) {
      launchTimer = 0;
      launchStatus = false;
      Serial.println("Launch Status: Ignition!");
      OCA.digitalWrite(MOSFET1, HIGH);
      delay(1000);
      OCA.digitalWrite(MOSFET1, LOW);
      Serial.println("Launch Status: Take Off!");
    }
  } else {
    OCA.digitalWrite(MOSFET1, LOW);
    launchTimer = 0;
  }
}

void handle_OnConnect() {
  launchStatus = LOW;
  launchTimer = 0;
  Serial.println("Launch Status: Ready!");
  server.send(200, "text/html", SendHTML(launchStatus, 0));
}

void handle_launch() {
  launchStatus = HIGH;
  launchTimer = millis();
  Serial.println("Launch Status: Count Down Started");
  server.send(200, "text/html", SendHTML(true, 0));
}

void handle_abort() {
  launchStatus = LOW;
  Serial.println("Launch Status: ABORT");
  server.send(200, "text/html", SendHTML(false, 0));
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}
const char cd[] PROGMEM = R"rawliteral(
<p><div id="countdown"><div></p>
<script>
  var timeleft = 10;
  var downloadTimer = setInterval(function(){
    if(timeleft <= 0){
      clearInterval(downloadTimer);
      document.getElementById("countdown").innerHTML = "Ignition";
    } else {
      document.getElementById("countdown").innerHTML = "T-" + timeleft ;
    }
    timeleft -= 1;
  }, 1000);
</script>
)rawliteral";

String SendHTML(uint8_t launchstat, int countDown) {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>Rocket Launcher</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr += ".button {display: block;width: 200px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 36px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr += ".button-launch {background-color: green;}\n";
  ptr += ".button-launch:active {background-color: darkgreen;}\n";
  ptr += ".button-abort {background-color: red;}\n";
  ptr += ".button-abort:active {background-color: darkred;}\n";
  ptr += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr += "#countdown {font-size: 200px;color: #F00;display: inline-block;inline-size: 75%;background-color: #AAA;border: 10px solid #000;border-radius: 10px;font-weight: 900;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>Rocket Launcher</h1>\n";
  ptr += "<h3>Press LAUNCH to start countdown.</h3>\n";

  if (launchstat) {
    ptr += "<p>Launch Status: Count Down Started</p><a class=\"button button-abort\" href=\"/abort\">ABORT</a>\n";
    ptr += cd;
  } else {
    ptr += "<p>Launch Status: On Hold</p><a class=\"button button-launch\" href=\"/launch\">LAUNCH</a>\n";
  }

  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}