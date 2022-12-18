#include <SoftwareSerial.h>
#include <PZEM004Tv30.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "DHT.h"
#include <RTClib.h>
#include <Wire.h>

// Uncomment one of the lines below for whatever DHT sensor type you're using!
#define DHTPIN 13 //D7 ESP pin
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

/*Put your SSID & Password*/
const char* ssid = "Galaxy S10 Lite23a8";  // Enter SSID here
const char* password = "xdds4510";  //Enter Password here

ESP8266WebServer server(80); // Initialize the web server
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor
SoftwareSerial pzemSerial(3,1); // ESP8266 Rx Tx
PZEM004Tv30 pzem(pzemSerial);
RTC_DS3231 rtc;

float temp;
float hum;
float _voltage;
float _current;
float _power;
int light;
int hr;
int mnt;
int sec;
float inputLight;
int lampPin = 12; //D6 ESP pin
int fanPin = 14; //D5 ESP pin
int enginePin = 15; //D8 ESP pin
String lampRelay;
String fanRelay;
String engineRelay;
 
void setup(){
  
  pinMode(lampPin, OUTPUT);
  pinMode(fanPin, OUTPUT);
  pinMode(enginePin, OUTPUT);
  pzemSerial.begin(9600);
  delay(100);
  
  //DHT setup
  dht.begin();

  //RTC setup
  Wire.begin(5, 4); //(SDA(D1), SCL(D2))
  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));
  
  pzemSerial.println("Connecting to ");
  pzemSerial.println(ssid);

  //connect to your local wi-fi network
  WiFi.begin(ssid, password);

  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  
  pzemSerial.print(".");
  }
  pzemSerial.println("");
  pzemSerial.println("WiFi connected..!");
  pzemSerial.print("Got IP: "); 
  pzemSerial.println(WiFi.localIP());
   
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);

  server.begin();
  pzemSerial.println("HTTP server started");
  
}
void loop() {
  
  server.handleClient();
  
}

void handle_OnConnect() {
  
  //temperature control
  temp = dht.readTemperature(); // Gets the values of the temperature
  if(temp > 18.00){
    fanRelay = "ON";
    digitalWrite(fanPin, HIGH);
    }
  else {
    fanRelay = "OFF";
    digitalWrite(fanPin, LOW);
    }
  hum = dht.readHumidity(); // Gets the values of the humidity

  //light control
  inputLight = analogRead(A0); //A0 ESP pin
  light = 100.00 - (100.00*((float)inputLight/1024.00));
  if(inputLight > 512){
    lampRelay = "ON";
    digitalWrite(lampPin, HIGH);
    }
  else {
    lampRelay = "OFF";
    digitalWrite(lampPin, LOW);
    }

  //rtc control
  DateTime jetzt = rtc.now();
  hr = jetzt.hour();
  mnt = jetzt.minute();
  sec = jetzt.second();
  if(hr == 17 && mnt > 50 && mnt < 55){
    engineRelay = "ON";
    digitalWrite(enginePin, HIGH);
    }
  else{
    engineRelay = "OFF";
    digitalWrite(enginePin, LOW);
    }
  
  //power monitoring
  _voltage = pzem.voltage();
  _current = pzem.current();
  _power = pzem.power();

  //transmitting data to the server
  server.send(200, "text/html", SendHTML(temp, hum, light, lampRelay, fanRelay, engineRelay, _voltage, _current, _power)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float tempStat, float humStat, int lightStat, String relayOne, String relayTwo, String relayThree, float voltStat, float currStat, float powerStat){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta http-equiv=\"refresh\" content=\"5\" name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Mini Server Monitoring</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  ptr +="p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<div id=\"webpage\">\n";
  ptr +="<h1>Mini Server Monitoring</h1>\n";
  
  ptr +="<p>Temperature: ";
  ptr +=(int)tempStat;
  ptr +="&deg;C</p>";
  ptr +="<p>Humidity: ";
  ptr +=(int)humStat;
  ptr +="%</p>";
  ptr +="<p>Light: ";
  ptr +=lightStat;
  ptr +="%</p>";
  ptr +="<p>Lamp: ";
  ptr +=relayOne;
  ptr +="</p>";
  ptr +="<p>Fan: ";
  ptr +=relayTwo;
  ptr +="</p>";
  ptr +="<p>Engine: ";
  ptr +=relayThree;
  ptr +="</p>";
  ptr +="<p>Voltage: ";
  ptr +=voltStat;
  ptr +="V</p>";
  ptr +="<p>Current: ";
  ptr +=currStat;
  ptr +="A</p>";
  ptr +="<p>Power: ";
  ptr +=powerStat;
  ptr +="W</p>";
  
  ptr +="</div>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
