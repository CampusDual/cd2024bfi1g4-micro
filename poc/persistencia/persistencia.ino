#include <Wire.h>
#include <WiFi.h>
#include <Preferences.h>  // Para usar la memoria NVS

// Variables globales para almacenar las credenciales Wi-Fi
String ssid = "MOVISTAR_F390";       // SSID predeterminado
String password = "EXPf8CSuGxHSs9fXSiwG";   // Contraseña predeterminada

// Crear un objeto de la clase Preferences para usar NVS
Preferences preferences;

void setup() {
  Serial.begin(115200);
  delay(10);

  // Iniciar la memoria NVS
  preferences.begin("wifiCreds", false);  // Abre el espacio de memoria "wifiCreds" para escritura

  // Guardar el SSID y la contraseña predeterminada en la memoria NVS
  preferences.putString("ssid", ssid);     // Guardar SSID en NVS
  preferences.putString("password", password); // Guardar contraseña en NVS

  Serial.println("Credenciales Wi-Fi guardadas en la memoria NVS.");

  // Recuperar las credenciales Wi-Fi almacenadas en la NVS
  ssid = preferences.getString("ssid", "");      // Lee el SSID guardado
  password = preferences.getString("password", ""); // Lee la contraseña guardada

  Serial.print("Conectando a Wi-Fi: ");
  Serial.println(ssid);
  
  // Conectar al Wi-Fi usando las credenciales almacenadas
  WiFi.begin(ssid.c_str(), password.c_str());

  // Esperar hasta que se conecte al Wi-Fi
  int max_retries = 20;
  while (WiFi.status() != WL_CONNECTED && max_retries > 0) {
    delay(1000);
    Serial.print(".");
    max_retries--;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n¡Conexión Wi-Fi exitosa!");
    Serial.print("Dirección IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nNo se pudo conectar al Wi-Fi.");
  }

  preferences.end();  // Cierra el espacio de memoria NVS
}

void loop() {
  // Aquí puedes realizar otras tareas, como leer sensores o enviar datos.
  delay(10000);  // Espera 10 segundos entre ciclos
}
