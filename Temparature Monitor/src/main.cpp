/*********
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete project details at https://RandomNerdTutorials.com
*********/

// Import required libraries
#ifdef ESP32
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#else
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#endif
#include <OneWire.h>
#include <DallasTemperature.h>

#include <ESP8266mDNS.h>

#define HOST_NAME "fridge-tag"

// Data wire is connected to GPIO 4
#define ONE_WIRE_BUS 4

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);

// Variables to store temperature values
float temperatureC = 0;
float maxTemperature = -1000.0;
float minTemperature = 1000.0;
float maxTempAlarm = 25;
float minTempAlarm = 20;
bool isMaxAlarm = false;
bool isMinAlarm = false;

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 1000;

// Replace with your network credentials
const char *ssid = "admin";
const char *password = "domestos1216";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

float readDSTemperatureC()
{
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  if (tempC == -127.00)
  {
    Serial.println("Failed to read from DS18B20 sensor");
    return NAN;
  }
  else
  {
    Serial.print("Temperature Celsius: ");
    Serial.println(tempC);
  }
  return tempC;
}

// min function
void updateMinTemp(float tempC)
{
  // minTemperature = readDSTemperatureC();
  if (tempC < minTemperature)
  {
    minTemperature = tempC;
  }
  Serial.print("Min Temperature: ");
  Serial.println(minTemperature);
}

// max function
void updateMaxTemp(float tempC)
{
  // maxTemperature = readDSTemperatureC();
  if (tempC > maxTemperature)
  {
    maxTemperature = tempC;
  }
  Serial.print("Max Temperature: ");
  Serial.println(maxTemperature);
}
// alarm fun
void checkAlarm(float tempC)
{
  if (tempC > maxTempAlarm)
  {
    isMaxAlarm = true;
    Serial.println("Temperature max alarm triggered!");
  }
  else if (tempC < minTempAlarm)
  {
    isMaxAlarm = true;
    Serial.println("Temperature alarm min triggered!");
  }
  return;
}

// HTML page
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .ds-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>Fridge Tag</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="ds-labels">Temperature Celsius</span> 
    <span id="temperaturec">%TEMPERATUREC%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="ds-labels">Min Temp</span> 
    <span id="mintemperature">%MINTEMP%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="ds-labels">Max Temp</span> 
    <span id="maxtemperature">%MAXTEMP%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="ds-labels">Alarm</span> 
    <span id="alarm">%NO ALARM%</span>
    <sup class="units">&deg;C</sup>
  </p>
</body>
<script>
setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperaturec").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperaturec", true);
  xhttp.send();
}, 1000);

setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("mintemperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/mintemperature", true);
  xhttp.send();
}, 1000);

setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("maxtemperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/maxtemperature", true);
  xhttp.send();
}, 1000);

setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("alarm").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/alarm", true);
  xhttp.send();
}, 1000);

</script>
</html>)rawliteral";

// Replaces placeholder with DS18B20 values on the web page
String processor(const String &var)
{
  if (var == "TEMPERATUREC")
  {
    return String(temperatureC, 2); // 2 знаки після крапки
  }
  else if (var == "MINTEMP")
  {
    return String(minTemperature, 2);
  }
  else if (var == "MAXTEMP")
  {
    return String(maxTemperature, 2);
  }
  else if (var == "ALARM")
  {
    if (isMaxAlarm)
    {
      return String("Max Alarm Triggered");
    }
    else if (isMinAlarm)
    {
      return String("Min Alarm Triggered");
    }
    else
    {
      return String("No Alarm");
    }
  }
  return String();
}

void InitMDNS()
{
  if (!MDNS.begin(HOST_NAME))
  {
    Serial.println("Error starting mDNS");
  }
  MDNS.addService("http", "tcp", 80);
  Serial.println("mDNS started");
}

void setup()
{
  Serial.begin(115200);
  InitMDNS();
  Serial.println();

  sensors.begin();
  temperatureC = readDSTemperatureC();

  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", index_html, processor); });
  server.on("/temperaturec", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", String(temperatureC, 2)); });

  server.on("/mintemperature", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", String(minTemperature, 2)); });

  server.on("/maxtemperature", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", String(maxTemperature, 2)); });

  server.on("/alarm", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              String alarmStatus;
              if (isMaxAlarm)
              {
                alarmStatus = "Max Alarm Triggered";
              }
              else if (isMinAlarm)
              {
                alarmStatus = "Min Alarm Triggered";
              }
              else
              {
                alarmStatus = "No Alarm";
              }
              request->send(200, "text/plain", alarmStatus); });

  server.begin();
}

void loop()
{
  if ((millis() - lastTime) > timerDelay)
  {
    float newTemp = readDSTemperatureC();
    if (!isnan(newTemp))
    {
      temperatureC = newTemp;
      updateMinTemp(temperatureC);
      updateMaxTemp(temperatureC);
      checkAlarm(temperatureC);
    }
    lastTime = millis();
  }
  MDNS.update();
}
