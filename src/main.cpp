#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>

// Set your Static IP address
IPAddress local_IP(192, 168, 1, 1);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);

const char* ssid     = "ESP8266-Access-Point";
const char* password = "123456789";

ESP8266WebServer server(80);

void start_wifi_access_point();

void handle_endpoint_homepage();
void handle_endpoint_settings();

void store_wifi_credentials(String name, String pass);
void get_stored_wifi_name_and_pass(char* name, char* pass);
bool is_wifi_credentials_stored();

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB
  }

  if (!is_wifi_credentials_stored())
  {
    start_wifi_access_point();
  } else
  {
    char name[25];
    char pass[20];
    get_stored_wifi_name_and_pass(name, pass);
    WiFi.begin(name, pass);
    unsigned long start_of_wifi_connectnion = millis();
    Serial.println("Trying connection with");
    Serial.println(name);
    Serial.println(pass);
    while (WiFi.status() != WL_CONNECTED && millis() - start_of_wifi_connectnion < 15000) {
      delay(500);
      Serial.print(".");
    }
    if(WiFi.status() != WL_CONNECTED) {
      start_wifi_access_point();
    } else {
      Serial.println("Connected to internet!!!");
    }
  }

  server.on("/", HTTPMethod::HTTP_GET, handle_endpoint_homepage);
  server.on("/settings", HTTPMethod::HTTP_GET, handle_endpoint_settings);

  server.begin();
}

void loop()
{
  server.handleClient();
  // put your main code here, to run repeatedly:
}

void handle_endpoint_homepage()
{
  LittleFS.begin();

  File file = LittleFS.open("index.html", "r");

  if (!file) {
    Serial.println("could not open file for read");
    server.send(500, "application/json",
                "{\"error\":\"could not open file\"}");
  } else {
    server.streamFile<File>(file, "text/html");
    file.close();
  }

  LittleFS.end();
}

void handle_endpoint_settings()
{
  Serial.println("settings");
  String name = server.arg("name");
  String pass = server.arg("pass");
  Serial.println(name);
  Serial.println(pass);
  server.send(200, "text/plain", "ok");
  store_wifi_credentials(name, pass);
  ESP.reset();
}

void store_wifi_credentials(String name, String pass)
{
  EEPROM.begin(512);
  
  for (unsigned int i = 0; i < name.length(); i++)
  {
    EEPROM.put(i, name[i]);
  }
  EEPROM.put(name.length(), '\0');

  for (unsigned int i = 0; i < pass.length(); i++)
  {
    EEPROM.put(25 + i, pass[i]);
  }
  EEPROM.put(25 + pass.length(), '\0');

  EEPROM.end();
}

void get_stored_wifi_name_and_pass(char* name, char* pass)
{
  EEPROM.begin(512);
  for (unsigned int i = 0; i < 25; i++)
  {
    char c;
    EEPROM.get(i, c);
    name[i] = c;

    if (c == '\0')
    {
      break;
    }
  }

  for (unsigned int i = 0; i < 20; i++)
  {
    char c;
    EEPROM.get(25+i, c);
    pass[i] = c;

    if (c == '\0')
    {
      break;
    }
  }
}

bool is_wifi_credentials_stored() {
  EEPROM.begin(512);
  uint8_t val = EEPROM.read(0);
  if(val == 255) {
    return false;
  }

  return true;
}

void start_wifi_access_point() {
    Serial.print("Setting AP (Access Point)…");
    // Remove the password parameter, if you want the AP (Access Point) to be open
    WiFi.softAPConfig(local_IP, gateway, subnet);
    WiFi.softAP(ssid, password);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    // Print ESP8266 Local IP Address
    Serial.println(WiFi.localIP());
}