#include <Ticker.h>
Ticker blinker;

#define LED_COUNT 5
enum Leds {
  TopRed,
  TopNonRed,
  Middle,        // Green or Yellow
  BottomNonRed,  // Olive or Blue
  BottomRed
};
const int LEDpin[] = {25, 26, 19, 18, 17};
const bool LEDoff[] = {HIGH, HIGH, LOW, HIGH, HIGH};
bool LEDstate[] = {LOW, LOW, LOW, LOW, LOW};
int LEDfreq[] = {0, 0, 0, 0, 0};

void blink() {
  static int roller = 1;
  roller = roller % 16 + 1;
  for (int i = 0; i < LED_COUNT; i++) {
    if (LEDfreq[i] == 0) {
      if (LEDstate[i] != LEDoff[i]) {
        LEDstate[i] = LEDoff[i];
        digitalWrite(LEDpin[i], LEDstate[i]);
      }
    } else {
      if ((roller % LEDfreq[i]) == 0) {
        LEDstate[i] = !LEDstate[i];
        digitalWrite(LEDpin[i], LEDstate[i]);
      }
    }
  }
}

void LEDsetup() {
  for (int i = 0; i < LED_COUNT; i++) {
    pinMode(LEDpin[i], OUTPUT);
    digitalWrite(LEDpin[i], LEDoff[i]);
  }
  blinker.attach_ms(100, blink);
}

void LEDset(int led, int freq) { LEDfreq[led] = freq; }

void action(bool o) {
  if (o) {
    LEDset(BottomRed, 1);
    LEDset(BottomNonRed, 0);
  } else {
    LEDset(BottomRed, 0);
    LEDset(BottomNonRed, 16);
  }
}

void tach(bool at) {
  if (at) {
    blinker.attach_ms(100, blink);
  } else {
    blinker.detach();
    digitalWrite(LEDpin[BottomRed], LEDoff[BottomRed]);
    digitalWrite(LEDpin[BottomNonRed], !LEDoff[BottomNonRed]);
  }
}

void rec(bool on) {
  if (on) {
    digitalWrite(LEDpin[Middle], !LEDoff[Middle]);
  } else {
    digitalWrite(LEDpin[Middle], LEDoff[Middle]);
  }
}
