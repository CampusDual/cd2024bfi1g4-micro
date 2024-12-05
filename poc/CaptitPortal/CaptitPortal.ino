#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <Preferences.h>

String ssid = "";
String password = "";

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

class CaptivePortalHandler : public AsyncWebHandler {
public:
  CaptivePortalHandler() {}
  virtual ~CaptivePortalHandler() {}

  bool canHandle(AsyncWebServerRequest *request) {
    return true; 
  }

  void handleRequest(AsyncWebServerRequest *request) {
    request->send(200, "text/html", index_html);
  }
};

void setup() {
  Serial.begin(115200);

  preferences.begin("wifiCreds", false);

  ssid = preferences.getString("ssid", "");
  password = preferences.getString("password", "");
  ssid = "";
  password = "";

  if (ssid != "" && password != "") {
    WiFi.begin(ssid.c_str(), password.c_str());
    unsigned long startTime = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 15000) {
      delay(1000);
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConexión Wi-Fi exitosa!");
      Serial.print("IP del dispositivo: ");
      Serial.println(WiFi.localIP());
      return;
    }
  }

  WiFi.softAP(apSSID, apPassword, 1, false, 4);
  delay(500);

  Serial.println("\nConfigurando Access Point...");
  Serial.print("IP del AP: ");
  Serial.println(WiFi.softAPIP());

  dnsServer.start(53, "*", WiFi.softAPIP());

  startCaptivePortal();
}

void loop() {
  dnsServer.processNextRequest();
}

void startCaptivePortal() {
  server.addHandler(new CaptivePortalHandler());

  server.on("/setWiFi", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
      ssid = request->getParam("ssid", true)->value();
      password = request->getParam("password", true)->value();

      preferences.putString("ssid", ssid);
      preferences.putString("password", password);
      Serial.println("Credenciales Wi-Fi guardadas!");

      request->send(200, "text/html", "<html><body><h2>Reinicia el dispositivo para aplicar cambios.</h2></body></html>");

      delay(2000);
      ESP.restart();
    } else {
      request->send(400, "text/html", "<html><body><h2>Error: Credenciales no válidas.</h2></body></html>");
    }
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(200, "text/html", index_html);
  });

  server.begin();
}
