enum Buttons { check, cross, left, right, up, down, robot };
const int touchPin[] = { T0, T6, T4, T3, T5, T7, T8 };
const int threshold[] = { 60, 80, 100, 80, 80, 70, 80 };
byte button = 0;
bool buttons(int count, ...) {
  va_list args;
  va_start(args, count);
  for (int i = 0; i < count; i++) {
    if (bitRead(button, va_arg(args, int)) == 0) {
      va_end(args);
      return false;
    }
  }
  va_end(args);
  return true;
}
void touchset(void *b) {
  bitSet(button,int(b));
}
void CapTouchSetup() {
  for (int i = 0; i < 7; i++) {
    touchAttachInterruptArg(touchPin[i], touchset,(void *)i, threshold[i]);
  }
}

