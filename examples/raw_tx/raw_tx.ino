// обмен сырыми данными без CRC и буфера на отправку

#define G433_SLOW_MODE
#include <Gyver433.h>
Gyver433_TX<2, 0, G433_NOCRC> tx;

void setup() {
}

char data[] = "Hello from #xx"; // строка для отправки
byte count = 0;                 // счётчик для отправки

void loop() {
  // добавляем счётчик в строку
  data[12] = (count / 10) + '0';
  data[13] = (count % 10) + '0';
  if (++count >= 100) count = 0;

  // отправка данных типа byte*
  tx.write(data, sizeof(data));

  // отправка 10 раз в сек
  delay(100);
}
