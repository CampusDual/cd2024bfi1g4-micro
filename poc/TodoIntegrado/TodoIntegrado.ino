#include <Wire.h>
#include <SparkFun_SHTC3.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <Preferences.h>

SHTC3 mySHTC3;

String ssid = "";
String password = "";

const char* serverName = "https://api.thingspeak.com/update";
const char* apiKey = "P4ZJ935D2ZJV8HQ6";

const char* apSSID = "ESP32-Config";
const char* apPassword = "123456789";

AsyncWebServer server(80);
DNSServer dnsServer;

Preferences preferences;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Captive Portal</title>
</head>
<body>
  <h1>Welcome to the Captive Portal</h1>
  <p>Please configure your Wi-Fi below:</p>
  <form action="/setWiFi" method="POST">
    SSID: <input type="text" name="ssid"><br>
    Password: <input type="password" name="password"><br>
    <input type="submit" value="Save">
  </form>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);

  preferences.begin("wifiCreds", false);

  ssid = preferences.getString("ssid", "");
  password = preferences.getString("password", "");

  if (ssid != "" && password != "") {
    Serial.println("Intentando conectar a Wi-Fi...");
    WiFi.begin(ssid.c_str(), password.c_str());
    unsigned long startTime = millis();
    
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 15000) {
      delay(1000);
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Conexión Wi-Fi exitosa!");
      Serial.print("IP del dispositivo: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("No se pudo conectar a Wi-Fi. Iniciando AP...");
      WiFi.softAP(apSSID, apPassword);
      Serial.print("IP del AP: ");
      Serial.println(WiFi.softAPIP());
      startCaptivePortal();
    }
  } else {
    WiFi.softAP(apSSID, apPassword);
    Serial.println("Configurando Access Point...");
    Serial.print("IP del AP: ");
    Serial.println(WiFi.softAPIP());
    startCaptivePortal();
  }

  Wire.begin();
  if (mySHTC3.begin() != 0) {
    Serial.println("Error al inicializar el sensor SHTC3.");
    while (1);
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    SHTC3_Status_TypeDef result = mySHTC3.update();   
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
      Serial.println("Datos enviados correctamente a ThingSpeak");
    } else {
      Serial.println("Error al enviar datos a ThingSpeak");
    }
    http.end();
  }

  delay(20000);
}

void startCaptivePortal() {
  dnsServer.start(53, "*", WiFi.softAPIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", index_html);
  });

  server.on("/setWiFi", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
      ssid = request->getParam("ssid", true)->value();
      password = request->getParam("password", true)->value();

      preferences.putString("ssid", ssid);
      preferences.putString("password", password);
      Serial.println("Credenciales Wi-Fi guardadas en la memoria NVS.");

      request->send(200, "text/html", "<html><body><h2>Conexión Wi-Fi exitosa! Reiniciando...</h2></body></html>");

      WiFi.begin(ssid.c_str(), password.c_str());
      unsigned long startTime = millis();
      
      while (WiFi.status() != WL_CONNECTED && millis() - startTime < 15000) {
        delay(1000);
        Serial.println("Conectando a Wi-Fi...");
      }

      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Conexión Wi-Fi exitosa!");
        Serial.print("IP del dispositivo: ");
        Serial.println(WiFi.localIP());
      } else {
        Serial.println("No se pudo conectar a Wi-Fi. Iniciando AP...");
        WiFi.softAP(apSSID, apPassword);
        Serial.print("IP del AP: ");
        Serial.println(WiFi.softAPIP());
        startCaptivePortal();
      }

    } else {
      request->send(400, "text/html", "<html><body><h2>Error: Las credenciales no son válidas.</h2></body></html>");
    }
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(200, "text/html", index_html);
  });

  server.begin();
}
