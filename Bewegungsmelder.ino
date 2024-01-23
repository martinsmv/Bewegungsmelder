#include <WiFi.h>
#include <HTTPClient.h>
#include <UrlEncode.h>
#include <Arduino.h>
#include <ctime>
#include <NTPClient.h>
#include <WiFiUdp.h>

WiFiUDP udp;
const int localTimeZone = 1;
char timeStringBuff[30];

const char *ssid = "";
const char *password = "";

String phoneNumber = "";
String apiKey = "4740949";

const int trigPin = 2;  // Trig-Pin für den Ultraschallsensor
const int echoPin = 4;  // Echo-Pin für den Ultraschallsensor

int duration;
int distance;

WiFiServer server(80);
WiFiClient client;

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }

  strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
  Serial.println(timeStringBuff);
}

void handleRoot() {
  // Laufzeitmessung + Distanzberechnung durchführen
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2; // Lauzeit * Schallgeschwindigkeit(= 343m/s) / 2 um Distanz in cm zu berechnen

  // HTML-Antwort senden
  String response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
  response += "<html><body>";
  response += "<h1>Einparkhilfe</h1>";
  response += "<p id=\"distance\">Distanz: " + String(distance) + " cm</p>";
  response += "<script>setTimeout(function() { location.reload(); }, 1000);</script>";
  response += "</body></html>";
  client.print(response);
}

void sendMessage(String message) {
  // Data to send with HTTP POST
  String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);
  HTTPClient http;
  http.begin(url);

  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // Send HTTP POST request
  int httpResponseCode = http.POST(url);
  if (httpResponseCode == 200){
    Serial.print("Message sent successfully");
  }
  else{
    Serial.println("Error sending the message");
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);
  }

  // Free resources
  http.end();
}

void setup() {
  Serial.begin(115200);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // WLAN-Verbindung herstellen
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Server starten
  server.begin();
  Serial.println("Server started");

  // IP-Adresse des Servers in der seriellen Konsole anzeigen
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  configTime(localTimeZone * 3600, 0, "pool.ntp.org", "time.nist.gov");
}

void loop() {
  client = server.available();
  if (client) {
    Serial.println("New client connected");
    handleRoot();
    delay(200);
    client.stop();
    Serial.println("Client disconnected");
  }

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Zeit bis zum Eintreffen des Echos messen
  unsigned long duration = pulseIn(echoPin, HIGH);

  // Distanz berechnen (Annahme: Schallgeschwindigkeit beträgt 343 m/s)
  float distance = (duration * 0.0343) / 2;

  // Distanz über die serielle Schnittstelle ausgeben
  Serial.print("Distanz: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance <= 82) {
    printLocalTime(); // Wenn benötigt, wieder hinzufügen
    String message = "Jemand hat dein Zimmer am " + String(timeStringBuff) + " betreten";
    sendMessage(message);
    delay(10000);
  }

  delay(1000);
}
