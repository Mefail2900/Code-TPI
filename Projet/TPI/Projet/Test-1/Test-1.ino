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
//////////////------------------------------------------------------------------------------------------------------------------Condition plant
bool plant1 = true;
bool plant2 = false;


// Assign output variables to GPIO pins
//const int output25 = 25;

const int levelWatering = 20;

int timeWatering = 8000;// the time starts with 8 sec , time for watering, 1000 = 1 second 

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 

// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;
/*///////////////////////////////////////----------------- to finish 
void checkPlantInventory() {
  File file = SPIFFS.open("/plantInventory.json", "r");
  if (!file) return;

  StaticJsonDocument<1024> doc;
  if (deserializeJson(doc, file)) {
    file.close();
    return;
  }
  file.close();

  auto root = doc.as<JsonVariant>();
  if (root.is<JsonArray>()) {
    for (JsonObject p : root.as<JsonArray>()) {
      int id = p["ID"];
      if (id == 1)
      {
         plant1 = true;
      }
      if (id == 2) 
      {
        plant2 = true;
      }
    }
  } else if (root.is<JsonObject>()) {
    int id = root["ID"];
    if (id == 1) plant1 = true;
    if (id == 2) plant2 = true;
  }
}*//*///////////////////////////////////////----------------- to finish */


void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(output26, OUTPUT);

 // pinMode(output25, OUTPUT);

  // Set outputs to LOW
  digitalWrite(output26, HIGH);

  //digitalWrite(output25, LOW);

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

//checkPlantInventory();///open after 
}

void moistureSensor(){
  int raw = analogRead(SensorPin);


  moisturePercent = map(raw, airValue, waterValue, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);
}
void serverWeb(){
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
              //----------------------------------------------------Add plant
        if (header.indexOf("POST /addPlant") >= 0) {
        // read http header
        while (client.available()) {
          String line = client.readStringUntil('\n');
          if (line.length() <= 2) break;
        }

        // read the body of the forum 
        String body = "";
        while (client.available()) body += (char)client.read();

        // extract the type on percent 
        String type = "", percentStr = "";
        int typePos = body.indexOf("type=");
        int percentPos = body.indexOf("percent=");

        if (typePos != -1 && percentPos != -1) {
          type = body.substring(typePos + 5, body.indexOf("&"));
          percentStr = body.substring(percentPos + 8);
          type.replace('+', ' ');
        }

        int percent = percentStr.toInt();
        createPlant(type, percent);  // call with value 

        // Redirection
        client.println("HTTP/1.1 303 See Other");
        client.println("Location: /");
        client.println("Connection: close");
        client.println();
        break;
      }
         
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
          digitalWrite(output26, LOW);
          //Write the blush logs in the Json file to keep track 
          WriteLogJson1();
          delay(timeWatering);
          digitalWrite(output26, HIGH);
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

      // Get the vaulue of newTime
      int pos = body.indexOf("newTime=");
      if (pos != -1) {
        String value = body.substring(pos + 8);
        int t = value.toInt();
        if (t >= 1 && t <= 60) {
          timeWatering = t * 1000; // convert seconds in milliseconds 
          Serial.print("Temps changé à : ");
          Serial.println(timeWatering);
        } else {
          Serial.println("Valeur hors limite");
        }
      }

      //desn back to the index page 
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





            // Display the HTML web page
            
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".moistureText{font-size:3rem;color:#28a745;font-weight:bold;margin:0;}");
            client.println(".button {background-color: green;}");
            client.println(".buttonRed {background-color: red;color: white;} </style></head>");
            
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

            //Show the button "Arroser" on green, if the user click the button the color and the text of the button wil change on red and the text will be "Arrosage" made AI 
            client.println(
              "<p><button style='background:#4CAF50;color:#fff;padding:16px 40px;font-size:30px;border:none;cursor:pointer'"
              " onclick=\"(b=>{b.innerText='Arrosage';b.style.background='red';b.disabled=true;"
              "fetch('/water').then(()=>{b.innerText='Arroser';b.style.background='#4CAF50';b.disabled=false;refresh();});})(this)\">"
              "Arroser</button></p>"
            );

           
            client.print("<p>Le dernier arrosage est :  ");
            readLogsJson1(client);
            client.println(" </p>");

            //--------------------------------------------------------------------------------------------------------------------------------------------------Add new plant 
          
         if(!plant1 || !plant2 ){
            client.println("<h3>Nouvelle plante</h3>");
            client.println("<form action='/addPlant' method='POST'>");
            client.println("<label for='type'>Plante</label>");
            client.println("<input type='text' id='type' name='type' required>");
            client.println("<label for='type'> Pourcentage de cycles arrosage actives</label>");
            client.println("<input type='number' id='percent'  name='percent' required><br><br>");
            client.println("<input type='submit' value='Charger' style='padding:6px 16px;'>");
            client.println("</form></div>");
          }
          else{
            client.println("<h3>Maximum de plante ajoute</h3>");

          }
           

          if(plant1){
           
           readInventoryPlant1(client);
           
          }
          if(plant2){
            client.println("<h3>Modification plante  2</h3>");
            client.println("<form action='/modifyPlant1' method='POST'>");
            client.println("<label for='name'>Plante</label>");
            client.println("<input type='text' id='name'value='Mint' name='name' required>");
            client.println("<label for='type'> Pourcentage de cycles arrosage actives</label>");
            client.println("<input type='number' id='humidity' value='20' name='humidity' required><br><br>");
            client.println("<input type='submit' value='Modifier' style='padding:6px 16px;'>");
            client.println("</form></div>");
          }
          if(!plant1 && !plant2){
            client.println("<h3>Aucune plante ajoute </h3>");
          }

              //--------------------------------------------------------------------------------------------------------------------------------------------------Add new plant

            moistureSensor();

            // ----------  Script JS, code that refresh the values of the page without refreshing the page   made AI 
            client.println("<script>");

            //this function sends a request to "/moisture" after it converts it and its update to the element with a new value of the humidity 
            client.println("function refresh(){fetch('/moisture').then(r=>r.text()).then(v=>document.getElementById('hum').textContent=v+'%');}");

            //Automatically refresh the value of the humidity sensor every 1 second  without reloading the page
            client.println("setInterval(refresh,1000); refresh();");

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
    digitalWrite(output26, LOW);
    WriteLogJson1();
   // digitalWrite(output25, HIGH);
   
  }
  else{
    //Deactivate the pin of the pump water 
     digitalWrite(output26, HIGH);
    output26State = "off";

  }
}

void loop(){
  serverWeb();

  automaticWatering();


}
///---------------------------------------------Save history of plant 1
void WriteLogJson1(){
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
    doc["type"] = "Aromatic";
    doc["Humidity"] = moisturePercent;
    doc["date"] = dateTime;
    doc["pin"] = 26;
    

    // -------------------------------------------------- Write JSON to file
    File file = SPIFFS.open(logFile1, "w");
    serializeJsonPretty(doc, file);
    file.close();
  }
  
///---------------------------------------------Save history of plant 2
void WriteLogJson2(){
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

      doc["ID"] = 2;

      //take from json file 
      doc["type"] = "Aromatic";

      doc["Humidity"] = moisturePercent;
      doc["date"] = dateTime;
      doc["pin"] = 26;
      

      // -------------------------------------------------- Write JSON to file
      File file = SPIFFS.open(logFile2, "w");
      serializeJsonPretty(doc, file);
      file.close();
  }

void createPlant(const String& type, int percent) {
  if (plant1 && plant2) {
    Serial.println("Deux plantes déjà créées. Rien à faire.");
    return;
  }

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Erreur : impossible d'obtenir l'heure.");
    return;
  }

  char dateTime[20];
  strftime(dateTime, sizeof(dateTime), "%Y-%m-%d %H:%M", &timeinfo);

  DynamicJsonDocument newPlant(256);
  newPlant["type"] = type;
  newPlant["percent"] = percent;
  newPlant["date"] = dateTime;

  if (!plant1) {
    newPlant["ID"] = 1;
    plant1 = true;
  } else if (!plant2) {
    newPlant["ID"] = 2;
    plant2 = true;
  }

  DynamicJsonDocument doc(1024);
  File file = SPIFFS.open("/plantInventory.json", "r");
  if (file && !file.isDirectory()) {
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    if (error || !doc.is<JsonArray>()) {
      doc = DynamicJsonDocument(1024);
      doc.to<JsonArray>();
    }
  } else {
    doc.to<JsonArray>();
  }

  doc.as<JsonArray>().add(newPlant);

  file = SPIFFS.open("/plantInventory.json", "w");
  if (file) {
    serializeJsonPretty(doc, file);
    file.close();
    Serial.println("Nouvelle plante ajoutée.");
  } else {
    Serial.println("Erreur lors de l'écriture du fichier.");
  }
}

////------------------------------------------------------Inventory of plant


//for the id 1 ------------------------------------------------to finish 
void readInventoryPlant1(WiFiClient& client) {
File file = SPIFFS.open("/plantInventory.json", FILE_READ);
  if (!file || file.isDirectory()) {
    client.print("<p>Fichier introuvable</p>");
    return;
  }

  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    client.print("<p>Erreur de lecture JSON</p>");
    return;
  }

  JsonArray array = doc.as<JsonArray>();

  for (JsonObject plant : array) {
    if (plant["ID"] == 1) {
      const char* type = plant["type"];
      int percent = plant["percent"];

      client.println("<h3>Modification plante 1</h3>");
      client.println("<form action='/modifyPlant1' method='POST'>");

      client.println("<label for='type'>Plante</label>");
      client.print("<input type='text' id='type' value='");
      client.print(type);
      client.println("' name='type' required>");

      client.println("<label for='percent'> Pourcentage de cycles arrosage actives</label>");
      client.print("<input type='number' id='percent' value='");
      client.print(percent);
      client.println("' name='percent' required><br><br>");

      client.println("<input type='submit' value='Modifier' style='padding:6px 16px;'>");
      client.println("</form></div>");

      // Pour debug
      Serial.println("Formulaire plante 1 généré avec succès");
      Serial.print("type: "); Serial.println(type);
      Serial.print("percent: "); Serial.println(percent);

      break; 
    }
  }
}
//---------------------------------------------------------------------------

//for the id 2 
void readInventoryPlant2(WiFiClient client){
      //Open the file and reads it 
      File file = SPIFFS.open("/plantInventory.json", FILE_READ);
      
      if (!file || file.isDirectory()) {
        client.print("Fichier introuvable");
        return;
      }
      //Extract the value of the json file into variables
    
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    if (error) {
      client.print("Erreur de lecture JSON");
      file.close();
      return;
    }
  JsonArray array = doc.as<JsonArray>();  

  for (JsonObject plant : array) {        
    int id = plant["ID"];
    const char* type = plant["type"];
    int percent = plant["percent"];

    client.print("<p>ID : ");
    client.print(id);
    client.print(" | Plante : ");
    client.print(type);
    client.print(" | percent : ");
    client.print(percent);
    client.println("</p>");
  }
      
}

  void readLogsJson1(WiFiClient client) {
      //Open the file log.json and reads it 
      File file = SPIFFS.open("/log1.json", FILE_READ);
      
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

       const char* type = doc["type"];
      int humidity = doc["Humidity"];
      const char* date = doc["date"];
      int percent = doc["percent"];

      //show in the page 

      client.print("Plante : ");
      client.print(type);
      client.print(", Humidite : ");
      client.print(humidity);
      client.print("%, Date : ");
      client.print(date);
      client.print(" Pourcent d'activation arrosage : ");
      client.print(percent);
      client.print("%");

      file.close();
  }
    // Reads and prints the log content to the serial monitor
  void readLogsJson2(WiFiClient& client) {
      //Open the file log.json and reads it 
      File file = SPIFFS.open("/log2.json", FILE_READ);
      
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

 
         const char* type = doc["type"];
      int humidity = doc["Humidity"];
      const char* date = doc["date"];
      int percent = doc["percent"];

      //show in the page 

          client.print("Plante : ");
      client.print(type);
      client.print(", Humidite : ");
      client.print(humidity);
      client.print("%, Date : ");
      client.print(date);
      client.print(" Pourcent d'activation arrosage : ");
      client.print(percent);
      client.print("%");

      file.close();
  }


