// обмен сырыми данными без CRC и буфера на отправку

#define G433_SLOW_MODE
#include <Gyver433.h>
Gyver433_RX<2, 20, G433_NOCRC> rx;  // буфер на приём нужен!

void setup() {
  Serial.begin(9600);
}

void loop() {
  if (rx.tickWait()) {
    Serial.write(rx.buffer, rx.size);
    Serial.println();
  }
}