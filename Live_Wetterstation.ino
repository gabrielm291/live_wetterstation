#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

// WLAN-Zugangsdaten
const char* ssid = "";        // <-- dein WLAN
const char* password = "";      // <-- WLAN-Passwort

WebServer server(80);

void handleRoot() {
  String page = R"====(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Temperatur Live</title>
<style>
body {font-family:Arial; text-align:center; background:#1e1e2f; color:white; margin:0; padding:20px;}
.card {background:#2c2c3e; padding:20px; margin:20px auto; border-radius:10px; width:250px;}
.value {font-size:50px; font-weight:bold; margin-top:10px;}
</style>
</head>
<body>
<h1>Live Temperatursensor</h1>
<div class="card">Temperatur<div id="temp" class="value">--</div></div>
<div class="card">Luftfeuchtigkeit<div id="hum" class="value">--</div></div>
<script>
function updateData() {
  fetch("/data")
    .then(r => r.json())
    .then(data => {
      document.getElementById("temp").innerHTML = data.temp + " °C";
      document.getElementById("hum").innerHTML = data.hum + " %";
    })
    .catch(err => console.log("Fehler beim Laden der Daten:", err));
}
updateData();
setInterval(updateData, 1000); // 1 Sekunde Aktualisierung
</script>
</body>
</html>
)====";
  server.send(200, "text/html", page);
}

void handleData() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (isnan(t) || isnan(h)) { t = 0; h = 0; }
  String json = "{\"temp\":" + String(t) + ",\"hum\":" + String(h) + "}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  delay(2000);

  Serial.print("Verbinde mit WLAN: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 40) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WLAN verbunden!");
    Serial.print("IP-Adresse ESP32: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("Verbindung fehlgeschlagen! Bitte prüfen.");
    while(true) { delay(1000); } // Stoppt hier
  }

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
  Serial.println("Webserver gestartet!");
}

void loop() {
  server.handleClient();
}