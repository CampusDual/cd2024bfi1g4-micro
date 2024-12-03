#include <WiFi.h>

const char* ssid = "";
const char* password = "";

void wifiTask(void *param){
  Serial.begin(115200);
  delay(10);

  Serial.println("\nConectando al Wi-Fi...");
  WiFi.begin(ssid, password);

  int max_retries = 20; 
  while (WiFi.status() != WL_CONNECTED && max_retries > 0) {
    delay(1000);
    Serial.print(".");
    max_retries--;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n¡Conexión exitosa!");
    Serial.print("Dirección IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nNo se pudo conectar al Wi-Fi.");
    Serial.println("Revisa el nombre y la contraseña.");
  }
}

void setup() {
}

void loop() {
}