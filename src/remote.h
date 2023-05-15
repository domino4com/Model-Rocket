
//////////////////////////////////////////////
//        RemoteXY include library          //
//////////////////////////////////////////////

// RemoteXY select connection mode and include library 
#define REMOTEXY_MODE__ESP32CORE_BLE
#include <BLEDevice.h>

#include <RemoteXY.h>

// RemoteXY connection settings 
#define REMOTEXY_BLUETOOTH_NAME "Rocket"


// RemoteXY configurate  
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] =   // 61 bytes
  { 255,2,0,243,0,54,0,16,31,1,67,5,0,0,63,5,122,26,81,3,
  131,2,11,59,22,2,26,67,5,0,33,63,5,96,26,81,67,5,0,5,
  63,5,122,26,81,1,0,12,49,39,39,2,31,69,120,101,99,117,116,101,
  0 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  uint8_t activity; // =0 if select position A, =1 if position B, =2 if position C, ... 
  uint8_t execute; // =1 if button pressed, else =0 

    // output variables
  char info1[81];  // string UTF8 end zero 
  char feedback[81];  // string UTF8 end zero 
  char info2[81];  // string UTF8 end zero 

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0 

} RemoteXY;
#pragma pack(pop)

/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////

#include "esp_log.h"
// The total number of bytes (not messages) the message buffer will be able to
// hold at any one time.
#define LOG_MSG_BUF_SIZE 1024
// The size, in bytes, required to hold each item in the message,
#define LOG_MSG_ITEM_SIZE 512

static vprintf_like_t orig_esp_log = NULL;
#define MIN(a, b) ((a) < (b) ? (a) : (b))

int ble_log(const char *fmt, va_list l) {
  static char buf[LOG_MSG_ITEM_SIZE];
  int ret;

  /* Send remote log */
  ret = vsnprintf(buf, LOG_MSG_ITEM_SIZE, fmt, l);
  Serial.print(">>>>>>>");
  Serial.println(buf);
  if (ret > 0) {
    memset(RemoteXY.info2, '\0', sizeof(RemoteXY.info2));
    strncpy(RemoteXY.info2, buf, MIN(sizeof(RemoteXY.info2) - 1, ret));
  }

  /* Also use existing logging mechanism */
  if (orig_esp_log) orig_esp_log(fmt, l);

  // Write to stdout
  return vprintf(fmt, l);

  return ret;
}

void log_state() {
  String status = "The rocket is ";
  if (stateRead(Prepared)) status += "Prepared,";
  if (stateRead(Armed)) status += "Armed,";
  if (stateRead(Upright)) status += "Upright,";
  if (stateRead(Completed)) status += "Completed,";
  if (status.endsWith(",")) status.remove(status.length() - 1);
  status += ".";
  strcpy(RemoteXY.info1, status.c_str());

  status = "Execute to ";
  switch (RemoteXY.activity) {
    case 0:
      status += "Format";
      break;
    case 1:
      status += "Flight";
      break;
    case 2:
      status += "Download";
      break;
    default:
      status = "A=Format B=Flight C=Download";
      break;
  }
  strcpy(RemoteXY.feedback, status.c_str());
}
