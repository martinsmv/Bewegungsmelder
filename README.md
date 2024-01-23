# EK ESP32 Bewegungsmelder

Martin Simov

19.1.2024

## E**inführung**

Dieses Projekt verwendet einen ESP32-Mikrocontroller mit einem Ultraschallsensor, um eine Einparkhilfe zu simulieren. Wenn die Distanz zu einem Hindernis unter einen bestimmten Schwellenwert fällt, wird über WhatsApp eine Benachrichtigung gesendet. Das Projekt integriert auch einen einfachen Webserver, der die gemessene Distanz auf einer HTML-Seite anzeigt.

## **Projektbeschreibung**

Das Ziel dieses Projekts ist es, eine Einparkhilfe mit einem Ultraschallsensor zu erstellen, die in Echtzeit die Distanz zu einem Hindernis misst. Wenn die gemessene Distanz einen vordefinierten Schwellenwert unterschreitet, wird eine Nachricht über WhatsApp gesendet, um den Benutzer zu informieren. Der ESP32 fungiert auch als Webserver und stellt eine HTML-Seite bereit, auf der die aktuelle Distanz angezeigt wird.

## **Theorie**

Das Projekt basiert auf der Verwendung eines Ultraschallsensors, der die Zeit misst, die ein Schallsignal benötigt, um von einem Hindernis reflektiert zu werden. Die gemessene Zeit wird dann in eine Distanz umgerechnet. Die WiFi-Fähigkeiten des ESP32 ermöglichen es, eine Verbindung zu einem WLAN-Netzwerk herzustellen und eine Nachricht über WhatsApp zu senden.

## **Arbeitsschritt**

Der Code verwendet eine WiFi-Verbindung, um sich mit einem Netzwerk zu verbinden, und startet einen Webserver auf dem ESP32. Der Ultraschallsensor wird verwendet, um die Distanz zu einem Hindernis zu messen. Wenn die Distanz unter einen Schwellenwert fällt, wird die lokale Zeit abgerufen, und eine Benachrichtigung über WhatsApp wird mit der aktuellen Zeit und einem Nachrichtentext gesendet. Der Webserver zeigt auch die aktuelle Distanz auf einer HTML-Seite an.

```cpp
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

	unsigned long lastNotificationTime = 0;

  if (distance <= 10) {
    unsigned long currentTime = millis();

    // Überprüfen, ob seit der letzten Benachrichtigung mindestens 10 Sekunden vergangen sind
    if (currentTime - lastNotificationTime >= 10000) {
        printLocalTime(); // Wenn benötigt, wieder hinzufügen
        String message = "Jemand hat dein Zimmer am " + String(timeStringBuff) + " betreten";
        sendMessage(message);

        // Aktualisieren Sie die Zeit der letzten Benachrichtigung
        lastNotificationTime = currentTime;
    }
}

  delay(1000);
}
```

## **Zusammenfassung**

Das Projekt nutzt die Kombination von Ultraschallsensorik, WiFi-Kommunikation und Webserver-Funktionalität des ESP32, um eine Einparkhilfe mit Benachrichtigungsfunktion zu implementieren. Es veranschaulicht die Integration von Hardware- und Netzwerkkomponenten für eine praktische Anwendung.

## **Quellen**

https://randomnerdtutorials.com/esp32-send-messages-whatsapp/
