#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>

#ifndef APSSID
#define APSSID "Test"
#define APPSK  "passwort"
#endif

/* Set these to your desired credentials. */
const char *ssid = APSSID;
const char *password = APPSK;

ESP8266WebServer server(80);

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
   connected to this access point to see it.
*/

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  LittleFS.begin();
  server.serveStatic("/", LittleFS, "/index.html");
  server.begin();
  Serial.println("HTTP server started");
  server.serveStatic("/iromin.js", LittleFS, "/iromin.js");
}

void loop() {
  server.handleClient();
}