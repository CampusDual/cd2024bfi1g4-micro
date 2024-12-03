#include <Arduino.h>

void persistenceTask(void *param);

void setup(){
  serial.begin(155200);

  Serial.println("Starting.....");
  xTaskCreatePinnedToCore(persistenceTask(void *param));

}

void loop(){
  delay(10000);
}