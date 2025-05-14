#include <WiFi.h>
#include <WebServer.h>
#include "html.h"

/* ----------- Configuration ----------- */
constexpr char SSID[]     = "NETGEAR69";
constexpr char PASSWORD[] = "fancyflower620";

constexpr uint8_t SensorPin = A0;          // GPIO pin connected to the capacitive soil‑moisture sensor
constexpr uint16_t HttpPort = 80;
constexpr uint32_t timeDelay = 1000;  // sensor refresh interval (ms)

int ledConnection = 2;

 const int buttonWeb = 26;


/* ------------------------------------- */

WebServer server(HttpPort);

int moisturePct = 0; // Latest moisture percentage (0‑100)

/* --- Forward declarations --- */
void handleRoot();
void handleMoisture();
/* ---------------------------- */

void setup()
{
    Serial.begin(115200);
    delay(100);

     pinMode(ledConnection, OUTPUT);

    // Connect to Wi‑Fi
    Serial.printf("Connecting to \"%s\"", SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASSWORD);

    digitalWrite(buttonWeb, LOW);
    
    uint32_t attempt = 0;
    
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
        Serial.print('.');
        if (++attempt > 120) {            // 30 s timeout → restart
            Serial.println("\nRestarting...");
            ESP.restart();
            
        }
        digitalWrite(ledConnection, LOW);
    }
    Serial.printf("\nConnected, IP address: %s\n", WiFi.localIP().toString().c_str());
  
   
    // Register HTTP endpoints
    server.on("/", handleRoot);
    server.on("/readMoisture", handleMoisture);
    server.begin();
}


void handleRoot()
{
    server.send_P(200, "text/html", IndexHtml);
}

void handleMoisture()
{
    server.send(200, "text/plain", String(moisturePct));
}

void loop()
{ 
    digitalWrite(ledConnection, HIGH);
    // Read sensor (12‑bit ADC on ESP32)
    int raw = analogRead(SensorPin);
    moisturePct = 100 - ((raw * 100) / 4095);

    server.handleClient();
    delay(timeDelay);
}
