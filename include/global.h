#ifndef __GLOBAL_H__
#define __GLOBAL_H__
#include <Arduino.h>
#define LOGTAG "Rocket"
#define FILENAME "data.bin"
#define FILEPATH "/" FILENAME

struct Record {
  unsigned long time;
  float accelX;
  float accelY;
  float accelZ;
  float temp;
  float pressure;
};
#endif



