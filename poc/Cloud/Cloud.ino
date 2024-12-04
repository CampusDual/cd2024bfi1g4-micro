#include <Wire.h>
#include <SparkFun_SHTC3.h>
#include <HTTPClient.h>
#include <WiFi.h>

SHTC3 mySHTC3;

const char* ssid = "";
const char* password = "";

const char* serverName = "https://api.thingspeak.com/update";
const char* apiKey = "";

void setup() {
    Serial.begin(115200);
    Serial.println("Intentando conectar a Wi-Fi...");

    WiFi.begin(ssid, password);
    Wire.begin();
    unsigned long startTime = millis();
    
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 15000) {
        delay(1000);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConexión Wi-Fi exitosa!");
        Serial.print("IP del dispositivo: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nNo se pudo conectar a Wi-Fi. Iniciando AP...");
    }

    if (mySHTC3.begin() != 0) {
      Serial.println("Error al inicializar el sensor SHTC3.");
      while (1); 
    }

    Serial.println("Sensor SHTC3 inicializado correctamente.");
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        SHTC3_Status_TypeDef result = mySHTC3.update();

      if(mySHTC3.lastStatus == SHTC3_Status_Nominal)
      {
        float temp = mySHTC3.toDegC();
        float humidity = mySHTC3.toPercent();

        Serial.print("Temperatura: ");
        Serial.print(temp);
        Serial.print(" °C, Humedad: ");
        Serial.print(humidity);
        Serial.println(" %");
        HTTPClient http;
        String url = String(serverName) + "?api_key=" + apiKey + "&field1=" + temp + "&field2=" + humidity;
        http.begin(url);
        int httpCode = http.GET();
        if (httpCode > 0) {
            Serial.print("HTTP Response code: ");
            Serial.println(httpCode);
            Serial.println("Datos enviados correctamente a ThingSpeak");
        } else {
            Serial.print("Error al enviar datos a ThingSpeak. Código: ");
            Serial.println(httpCode);
        }
        http.end();
      }


    } else {
        Serial.println("Wi-Fi desconectado. Reintentando...");
        WiFi.reconnect();
    }
  
    delay(20000); // 20 segundos entre mediciones
}
