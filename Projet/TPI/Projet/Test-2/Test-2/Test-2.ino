/*********
  Mefail Sulejmani 
*********/

// Load Wi-Fi library
#include <WiFi.h>

//load SPIFFS library
#include <SPIFFS.h>

//load arduino json library
#include <ArduinoJson.h>

#include <time.h>

#include "Historique.h"

// Replace with your network credentials
const char* ssid = "NETGEAR69";
const char* password = "fancyflower620";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output26State = "off";

constexpr uint8_t SensorPin = A0;          // GPIO pin connected to the capacitive soil‑moisture sensor

int moisturePercent = 0; // Latest moisture percentage (0‑100)

const int airValue = 3800;    // Value of the air
const int waterValue = 800;   // value of water 

// Assign output variables to GPIO pins
const int output26 = 26;

const int levelWatering = 20;

int timeWatering = 8000;// the time starts with 8 sec , time for watering, 1000 = 1 second 

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 

// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(output26, OUTPUT);

  // Set outputs to LOW
  digitalWrite(output26, LOW);


  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();


  
   if (!SPIFFS.begin(false)) {
    Serial.println("Failed to mount SPIFFS!");
    return;
  }

}

void moistureSensor(){
  int raw = analogRead(SensorPin);


  moisturePercent = map(raw, airValue, waterValue, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);
}
void serverWeb()
{
   WiFiClient client = server.available();   // Listen for incoming clients
  moistureSensor();
  
  
  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;

    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
       
        header += c;
         // ----------  Nouvelles routes API légères  ----------
        if (header.indexOf("GET /moisture") >= 0) {
          moistureSensor();                           // fresh value 
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/plain");
          client.println("Connection: close\n");
          client.print(moisturePercent);                  // % of the moisture sensor 
          break;
        }
        if (header.indexOf("GET /water") >= 0) {
          Serial.println("Arrosage déclenché via /water");
          output26State = "Arrosage";
          digitalWrite(output26, HIGH);
          delay(timeWatering);
          digitalWrite(output26, LOW);
          output26State = "off";
          client.println("HTTP/1.1 204 No Content"); // réponse vide
          client.println("Connection: close\n");
          break;
        }
        if (header.indexOf("POST /timeWater") >= 0) {
      // Lire jusqu’à la fin des headers
      while (client.available()) {
        String line = client.readStringUntil('\n');
        if (line.length() <= 2) break;
      }

      // Lire le contenu du formulaire
      String body = "";
      while (client.available()) body += (char)client.read();

      // Récupérer la valeur de newTime (ex. newTime=8)
      int pos = body.indexOf("newTime=");
      if (pos != -1) {
        String value = body.substring(pos + 8);
        int t = value.toInt();
        if (t >= 1 && t <= 60) {
          timeWatering = t * 1000; // convertir secondes → millisecondes
          Serial.print("Temps changé à : ");
          Serial.println(timeWatering);
        } else {
          Serial.println("Valeur hors limite");
        }
      }

      // Redirige vers la page d'accueil
      client.println("HTTP/1.1 303 See Other");
      client.println("Location: /");
      client.println("Connection: close");
      client.println();
      break;
    }

        
        // -----------------------------------------------------

        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");





            // turns the GPIO 26 on and off
            if (header.indexOf("GET /26/on") >= 0) {
              Serial.println("GPIO 26 on");
              output26State = "Arrosage";
    
              digitalWrite(output26, HIGH);
              delay(timeWatering);
              output26State = "off";
              digitalWrite(output26, LOW);
            } else if (header.indexOf("GET /26/off") >= 0) {
              Serial.println("GPIO 26 off");
              output26State = "off";
              digitalWrite(output26, LOW);
            }


            // Display the HTML web page
            
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".moistureText{font-size:3rem;color:#28a745;font-weight:bold;margin:0;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP32 Web Server</h1>");
            
           
            client.print("<p>Le temps d'arrosage est de ");
            client.print(timeWatering / 1000, 1);   // 1 décimale, 60000 ms = 1 min
            client.println(" secondes</p>");
           
            client.println("<form action=\"/timeWater\" method=\"POST\">");
            client.print("<label for=\"newTime\">Temps d'arrosage</label>");
            client.print("<input type=\"number\" id=\"newTime\" name=\"newTime\" min=\"1\" max=\"60\" required>");
            client.print("<input type=\"submit\" value=\"Changer\" style=\"margin-left: 10px;\">");
            client.print("</form>");

            client.print("<p id=\"hum\" class=\"moistureText\">");
            client.print(moisturePercent );
            client.print("%</p>");


            // If the output26State is off, it displays the ON button       
            if (output26State=="off") {
              client.println("<p><button class=\"button\" onclick=\"water()\">Arroser</button></p>");
            } else {
              client.println("<p><button class=\"button button2\" disabled>OFF</button></p>");
            } 
            client.print("<p>Le dernier arrosage est :  ");
           // readLogsJson(client);
            client.println(" </p>");

            moistureSensor();

            // ----------  Script JS, code that refresh the values of the page without refreshing the page  
            client.println("<script>");

            //this function sends a request to "/moisture" after it converts it and its update to the element with a new value of the humidity 
            client.println("function refresh(){fetch('/moisture').then(r=>r.text()).then(v=>document.getElementById('hum').textContent=v+'%');}");

            //Automatically refresh the value of the humidity sensor every 1 second  without reloading the page
            client.println("setInterval(refresh,1000); refresh();");

            
            //in this function when the button "Arrosage" is clicked he will send a request for activating the pump and after it will refresh the value of the humidity 
            client.println("function water(){fetch('/water').then(()=>refresh());}");

  
            client.println("</script>");
            
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();

  }
}
//This function checks if the humidity level is right 
void automaticWatering(){

  //This condition is made to check if the level of the humidity is correct
  if (moisturePercent < levelWatering ) {

    //Activate the pin of the pump of water 
    output26State = "Arrosage";
    digitalWrite(output26, HIGH);
    WriteLogJson();
   
  }
  else{
    //Deactivate the pin of the pump water 
     digitalWrite(output26, LOW);
    output26State = "off";
  }
}

void loop(){
  serverWeb();

  automaticWatering();

}

void WriteLogJson(){
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to get time from NTP server");
    return;
  }

  // Format the current date and time as a string
  char dateTime[20];
  strftime(dateTime, sizeof(dateTime), "%Y-%m-%d %H:%M", &timeinfo);
    
  DynamicJsonDocument doc(512);

    doc["ID"] = 1;
    doc["name"] = "Mint";
    doc["type"] = "Aromatic";
    doc["Humidity"] = moisturePercent;
    doc["date"] = dateTime;
    doc["pin"] = 26;
    

    // -------------------------------------------------- Write JSON to file
    File file = SPIFFS.open(logFile, "w");
    serializeJsonPretty(doc, file);
    file.close();
  }
  
// Reads and prints the log content to the serial monitor

  void readLogsJson(WiFiClient client) {
      //Open the file log.json and reads it 
      File file = SPIFFS.open("/log.json", FILE_READ);
      
      if (!file || file.isDirectory()) {
        client.print("Error file not found");
        return;
      }
      //Extract the value of the json file into variables
    
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, file);

    if (error) {
      client.print("Erreur de lecture JSON");
      file.close();
      return;
    }

      const char* name = doc["name"];
      const char* type = doc["type"];
      int humidity = doc["Humidity"];
      const char* date = doc["date"];

      //show in the page 

      client.print("Plante : ");
      client.print(name);
      client.print(", Type : ");
      client.print(type);
      client.print(", Humidite : ");
      client.print(humidity);
      client.print("%, Date : ");
      client.print(date);

      file.close();
  }
