/*********
  Mefail Sulejmani 
*********/

// Load Wi-Fi library
#include <WiFi.h>

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

int moisturePct = 0; // Latest moisture percentage (0‑100)


// Assign output variables to GPIO pins
const int output26 = 26;



const int timeWatering = 8000;// time for watering, 1000 = 1 second 

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
}

void muistureSensor(){
  int raw = analogRead(SensorPin);            //Sensor analog
  moisturePct = 100 - ((raw * 100) / 4095);   //calculator of the sensor
}
void serverWeb()
{
   WiFiClient client = server.available();   // Listen for incoming clients
  muistureSensor();
  
  
  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;

    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
         // ----------  Nouvelles routes API légères  ----------
        if (header.indexOf("GET /moisture") >= 0) {
          muistureSensor();                           // fresh value 
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/plain");
          client.println("Connection: close\n");
          client.print(moisturePct);                  // % of the moisture sensor 
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
           
            client.print("<p id=\"hum\" class=\"moistureText\">");
            client.print(moisturePct );
            client.print("%</p>");


            // If the output26State is off, it displays the ON button       
            if (output26State=="off") {
              client.println("<p><button class=\"button\" onclick=\"water()\">Arroser</button></p>");
            } else {
              client.println("<p><button class=\"button button2\" disabled>OFF</button></p>");
            } 
               
            muistureSensor();

            // ----------  Script JS, code that refresh the values of the page without refreshing the page  
            client.println("<script>");

            //this function sends a request to "/moisture" after it converts it and its update to the element with a new value of the humidity 
            client.println("function refresh(){fetch('/moisture').then(r=>r.text()).then(v=>document.getElementById('hum').textContent=v+'%');}");

            //Automatically refresh the value of the humidity sensor every 1,2 second  without reloading the page
            client.println("setInterval(refresh,1200); refresh();");

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
void automaticWatering(){

  if (moisturePct < 20 && output26State == "off") {
    Serial.println("Arrosage automatique déclenché !");
    output26State = "Arrosage";
    digitalWrite(output26, HIGH);
    delay(timeWatering);
    digitalWrite(output26, LOW);
    output26State = "off";
  }
}

void loop(){
 serverWeb();

    automaticWatering();
}