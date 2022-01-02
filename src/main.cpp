// Libaries
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <Adafruit_NeoPixel.h>

// Define name and password from Acces Point
#ifndef APSSID
#define APSSID "myFancyLight"
#define APPSK "passwort"
#endif
#define LEDPIN 2
#define NUMLED 5

// Tell neopixel libary used pin and amount of leds
Adafruit_NeoPixel leds(NUMLED, LEDPIN, NEO_GRB + NEO_KHZ800);

// Set these to your desired credentials.
const char *ssid = APSSID;
const char *password = APPSK;

// Create Server
ESP8266WebServer server(80);

// create config
struct Config
{
  String fancyMode;
  int red = 255;
  int grn = 255;
  int blu = 255;
};
int rainbowSpeed = 500;

const char *nameConfigFile = "/config.txt";
Config config; // <- global configuration object

void setLedColor(int red, int grn, int blu, int speed)
{
  for (int i = 0; i < leds.numPixels(); i++)
  { // For each pixel...
    // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    // Here we're using a moderately bright green color:
    leds.setPixelColor(i, leds.Color(red, grn, blu));
    leds.show();  // Send the updated pixel colors to the hardware.
    delay(speed); // Pause before next pass through loop
  }
}

uint32_t Wheel(byte WheelPos)
{
  if (WheelPos < 85)
  {
    return leds.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
  else if (WheelPos < 170)
  {
    WheelPos -= 85;
    return leds.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  else
  {
    WheelPos -= 170;
    return leds.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

void rainbowCycle(uint8_t wait)
{
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++)
  { // 5 cycles of all colors on wheel
    for (i = 0; i < leds.numPixels(); i++)
    {
      leds.setPixelColor(i, Wheel(((i * 256 / leds.numPixels()) + j) & 255));
    }
    server.handleClient();
    if (config.fancyMode == "false")
    {
      break;
    }
    leds.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait)
{
  uint16_t i, j;

  for (j = 0; j < 256; j++)
  {
    for (i = 0; i < leds.numPixels(); i++)
    {
      leds.setPixelColor(i, Wheel((i + j) & 255));
    }
    leds.show();
    delay(wait);
    server.handleClient();
    if (config.fancyMode != "true")
    {
      break;
    }
  }
}

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
  setLedColor(config.red, config.grn, config.blu, 0);
}

void getConfig()
{
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

  // Init leds
  leds.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  setLedColor(0, 0, 0, 0);

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
  if (config.fancyMode == "true")
  {
    rainbowCycle(10);
  }
  else
  {
    setLedColor(config.red, config.grn, config.blu, 0);
  }
}