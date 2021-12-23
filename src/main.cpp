// Libaries
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>

// Define name and password from Acces Point
#ifndef APSSID
#define APSSID "myFancyLight"
#define APPSK  "passwort"
#endif

// Set these to your desired credentials.
const char *ssid = APSSID;
const char *password = APPSK;

// Create Server
ESP8266WebServer server(80);

// Variable declaration
String fancyMode; 
int red = 0;
int grn = 0;
int blu = 0;

// Function that gets colorstatus from server
void getColor() {
  String r = server.arg("r");
  String g = server.arg("g");
  String b = server.arg("b");
  fancyMode = server.arg("fancyMode");
  red = r.toInt();
  grn = g.toInt();
  blu = b.toInt();
  Serial.println("");
  Serial.println("New Color:");
  Serial.println("RGB: " + r + " " + g + " " + b + ", FancyMode: " + fancyMode);
  /*
  if (fancyMode == "false"){
    for (int i = 0; i < LEDS; i++){
      strip.setPixelColor(i,red, grn, blu);
      strip.show();
    }
  }*/

  server.send(200, "text/plain", "OK");
}

// Initial Routine
void setup() {

  // Give the Chip some time
  delay(1000);

  // Open Serial connection for debugging
  Serial.begin(115200);
  Serial.println();

  // Open AP
  Serial.print("Configuring access point...");
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  // Make the Flashstorage accessible
  LittleFS.begin();

  // Start Webserver and listen to requests
  server.serveStatic("/", LittleFS, "/index.html");
  server.serveStatic("/iromin.js", LittleFS, "/iromin.js");
  server.on("/sendColor", getColor);
  server.begin();
  Serial.println("HTTP server started");
}

// Programm loop
void loop() {
  server.handleClient();
}