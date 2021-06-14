// приём в прерывании. Отправляет пример demo_tx
#define G433_SLOW_MODE

#include <Gyver433.h>
Gyver433_RX<2, 20> rx;  // указали пин и размер буфера

void setup() {
  Serial.begin(9600);
  // взводим прерывания по CHANGE
  attachInterrupt(0, isr, CHANGE);
}

void isr() {
  rx.tick();  // тикер вызывается в прерывании
}

void loop() {
  // .gotData() вернёт true при получении корректных данных
  // и сам сбросится до следующего приёма
  // внутри gotData() встроен тик!
  if (rx.gotData()) {
    Serial.write(rx.buffer, rx.size);
    Serial.println();
  }

  // имитация загруженного кода
  delay(200);
}
