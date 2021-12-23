// Libaries
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>

// Define name and password from Acces Point
#ifndef APSSID
#define APSSID "myFancyLight"
#define APPSK "passwort"
#endif

// Set these to your desired credentials.
const char *ssid = APSSID;
const char *password = APPSK;

// Create Server
ESP8266WebServer server(80);

// create config
struct Config
{
  String fancyMode;
  int red;
  int grn;
  int blu;
};

const char *nameConfigFile = "/config.txt";
Config config; // <- global configuration object

// Loads the configuration from a file
void loadConfiguration(const char *filename, Config &config)
{

  Serial.println("Lade Konfiguration.");

  // Open file for writing
  File configFile = LittleFS.open(nameConfigFile, "r");
  if (!configFile)
  {
    Serial.println("Öffnen der Konfigurationsdatei zum lesen fehlgeschlagen.");
    return;
  }
  Serial.println("Konfigurationsdatei geöffnet.");

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use https://arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<256> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, configFile);
  if (error)
  {
    Serial.println(F("Failed to read file, usi"));
    return;
  }

  // Copy values from the JsonDocument to the Config
  config.red = doc["red"];
  config.grn = doc["grn"];
  config.blu = doc["blu"];
  config.fancyMode = doc["fancyMode"].as<String>();
  Serial.println("Konfiguration gelesen.");

  // Close the file (Curiously, File's destructor doesn't close the file)
  configFile.close();
  Serial.println("Konfigurationsdatei geschlossen.");
}

// Saves the configuration to a file
void saveConfig(const char *nameConfigFile, const Config &config)
{

  Serial.println("Speichere Konfiguration.");

  // Open file for writing
  File configFile = LittleFS.open(nameConfigFile, "w");
  if (!configFile)
  {
    Serial.println("Öffnen der Konfigurationsdatei zum beschreiben fehlgeschlagen");
    return;
  }
  Serial.println("Konfigurationsdatei geöffnet.");

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use https://arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<256> doc;

  // Set the values in the document
  doc["fancyMode"] = config.fancyMode;
  doc["red"] = config.red;
  doc["grn"] = config.grn;
  doc["blu"] = config.blu;

  // Serialize JSON to file
  if (serializeJson(doc, configFile) == 0)
  {
    Serial.println(F("Failed to write to file"));
    return;
  }
  Serial.println("Konfiguration geschrieben.");

  // Close the file
  configFile.close();
  Serial.println("Konfigurationsdatei geschlossen.");
}

// print content from configfile
void printFile(const char *nameConfigFile)
{
  // Open file for reading
  File configFile = LittleFS.open(nameConfigFile, "r");
  if (!configFile)
  {
    Serial.println("Öffnen der Konfigurationsdatei zum auslesen fehlgeschlagen");
    return;
  }

  // Extract each characters by one by one
  while (configFile.available())
  {
    Serial.print((char)configFile.read());
  }
  Serial.println();

  // Close the file
  configFile.close();
}

// Function that gets colorstatus from server
void setColor()
{
  String r = server.arg("r");
  String g = server.arg("g");
  String b = server.arg("b");
  config.fancyMode = server.arg("fancyMode");
  config.red = r.toInt();
  config.grn = g.toInt();
  config.blu = b.toInt();
  Serial.println("");
  Serial.println("New Color:");
  Serial.println("RGB: " + r + " " + g + " " + b + ", FancyMode: " + config.fancyMode);
  saveConfig(nameConfigFile, config);
  printFile(nameConfigFile);
  /*
  if (fancyMode == "false"){
    for (int i = 0; i < LEDS; i++){
      strip.setPixelColor(i,red, grn, blu);
      strip.show();
    }
  }*/
  server.send(200, "text/plain", "OK");
}

void getConfig() {
  Serial.println("Konfiguration wird vom Webserver abgerufen.");
  String json = "{\"fancyMode\":\"" + config.fancyMode + "\",\"red\":" + config.red + ",\"grn\":" + config.grn + ",\"blu\":" + config.blu + "}";
  server.send(200, "text/plain", json);
}

// Initial Routine
void setup()
{
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

  // Load config
  loadConfiguration(nameConfigFile, config);

  // Start Webserver and listen to requests
  server.serveStatic("/", LittleFS, "/index.html");
  server.serveStatic("/iromin.js", LittleFS, "/iromin.js");
  server.on("/sendColor", setColor);
  server.on("/getConfig", getConfig);
  server.begin();
  Serial.println("HTTP server started");
}

// Programm loop
void loop()
{
  server.handleClient();
}