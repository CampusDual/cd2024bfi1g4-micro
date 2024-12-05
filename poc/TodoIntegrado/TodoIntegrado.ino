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
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>Captive Portal</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background-color: #f7f9fc;
      color: #333;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      margin: 0;
    }
    .container {
      background: #fff;
      border: 1px solid #ddd;
      border-radius: 8px;
      box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
      padding: 20px;
      max-width: 400px;
      width: 100%;
      text-align: center;
    }
    h1 {
      font-size: 1.8em;
      color: #333;
      margin-bottom: 0.5em;
    }
    p {
      color: #555;
      margin-bottom: 1.5em;
    }
    form {
      display: flex;
      flex-direction: column;
    }
    input[type="text"], input[type="password"] {
      font-size: 1em;
      padding: 10px;
      margin-bottom: 15px;
      border: 1px solid #ccc;
      border-radius: 4px;
      width: 100%;
      box-sizing: border-box;
    }
    input[type="submit"] {
      background-color: #4CAF50;
      color: white;
      border: none;
      padding: 10px 15px;
      font-size: 1em;
      border-radius: 4px;
      cursor: pointer;
      transition: background-color 0.3s;
    }
    input[type="submit"]:hover {
      background-color: #45a049;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>Bienvenido</h1>
    <p>Configuracion del wifi</p>
    <form action="/setWiFi" method="POST">
      <input type="text" name="ssid" placeholder="Nombre WiFi" required>
      <input type="password" name="password" placeholder="Contraseña" required>
      <input type="submit" value="Save">
    </form>
  </div>
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
    
    WiFiConecction();

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

      request->send(200, "text/html", "<html><body><h2>Conexion Wi-Fi exitosa! Reiniciando...</h2></body></html>");

      WiFi.begin(ssid.c_str(), password.c_str());
      
      WiFiConecction();

    } else {
      request->send(400, "text/html", "<html><body><h2>Error: Las credenciales no son validas.</h2></body></html>");
    }
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(200, "text/html", index_html);
  });

  server.begin();
}

void WiFiConecction() {

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
}
