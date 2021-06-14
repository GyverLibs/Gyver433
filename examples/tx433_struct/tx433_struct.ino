// передача структуры данных

#define G433_SLOW_MODE
#include <Gyver433.h>
Gyver433_TX<2, 10> tx;  // указали пин и размер буфера

// формат пакета для отправки
struct dataPack {
  byte counter;
  byte randomNum;
  int analog;
  uint32_t time;
};
dataPack data;

void setup() {
  Serial.begin(9600);
}

void loop() {
  data.counter++;                 // тут счётчик
  data.randomNum = random(256);   // случайное число
  data.analog = analogRead(0);    // тут ацп
  data.time = millis();           // тут миллис

  tx.sendData(data);
  
  Serial.println("Transmit:");
  Serial.println(data.counter);
  Serial.println(data.randomNum);
  Serial.println(data.analog);
  Serial.println(data.time);
  Serial.println();

  delay(1000);
}
