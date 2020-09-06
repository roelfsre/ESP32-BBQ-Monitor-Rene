/*
* ESPBBQ
* Made by Roly van Leersum en René Roelfsema
* Github URL: https://github.com/roly197/ESPBBQ
* 
* Pinout details:
* 
*      ESP32       
*    DEVKIT V1
*    |------|    R1
*    | 3.3V |_--^^^^---|
*    |      |          |
*    |   VP |----------|
*    |      |  R0(NTC) |
*    |  GND |---^^^^---|
*    |------|
* 
*     NTC B3950 Thermistor
*     the formula for temp in kelvin is
*                     1
*     T = ----------------------------
*         1/To + (1/beta) * ln(Rt/Ro)
*    
*     To determine Beta:
*     R(t0) = resistance @ 273.15 Kelvin; Ice water (or 0 Celcius) 
*     R(t1) = resistance @ 373.15 Kelvin: Boiling water (or 100 Celcius)
*     Then: 
*         Beta = ln(Rt1/Rt0)/(1/t1-1/t0)
*    
*     https://en.wikipedia.org/wiki/Thermistor
*/

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <AsyncTCP.h>
//#include <ESPAsyncWebServer.h>
//#include <ESPDash.h>

//AsyncWebServer server(80);
WebServer server(80); 

const char* ssid = "BR"; // Your WiFi SSID
const char* password = "SamenJay"; // Your WiFi Password

float Tc;

int ThermistorPin;
double adcMax, Vs;

double R1 = 32600.0;   // voltage divider resistor value (33k)
double Beta = 3950.0;  // Beta value
double To = 298.15;    // Temperature in Kelvin for 25 degree Celsius
double Ro = 81000.0;   // Resistance of Thermistor at 25 degree Celsius


void setup() {
    Serial.begin(115200);

    ThermistorPin = A0;
    adcMax = 4095.0;   // ADC resolution 10-bit (0-1023)
    Vs = 3.3;          // supply voltage

    //Setup Wifi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.printf("WiFi Failed!\n");
        return;
    }
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    server.on("/", handle_OnConnect);
    server.onNotFound(handle_NotFound);

    server.begin();
    Serial.println("HTTP server started");
/*    //Setup Dash (Web framework)
*    ESPDash.init(server);   // Initiate ESPDash and attach your Async webserver instance
*    ESPDash.addTemperatureCard("temp1", "Meat Temp", 0, 0);
*    server.begin();
}
*/
}
void loop() {
  server.handleClient();
}

void handle_OnConnect() {
      double Vout, Rt = 0;
      double T, Tc, Tf = 0;

      Vout = analogRead(ThermistorPin) * Vs/adcMax;
      Rt = R1 * Vout / (Vs - Vout);
      T = 1/(1/To + log(Rt/Ro)/Beta);  // Temperature in Kelvin
      Tc = T - 273.15;                 // Celsius
      Tf = Tc * 9 / 5 + 32;            // Fahrenheit
      Serial.println(Tc);
//      ESPDash.updateTemperatureCard("temp1", int (Tc));
      Serial.println(analogRead(ThermistorPin));
      server.send(200, "text/html", SendHTML(Tc));
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float Tc){
  String ptr = "<!DOCTYPE html>";
  ptr +="<html>";
  ptr +="<head>";
  ptr +="<title>BBQ Sensor</title>";
  ptr +="<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  ptr +="<link href='https://fonts.googleapis.com/css?family=Open+Sans:300,400,600' rel='stylesheet'>";
  ptr +="<style>";
  ptr +="html { font-family: 'Open Sans', sans-serif; display: block; margin: 0px auto; text-align: center;color: #444444;}";
  ptr +="body{margin: 0px;} ";
  ptr +="h1 {margin: 50px auto 30px;} ";
  ptr +=".side-by-side{display: table-cell;vertical-align: middle;position: relative;}";
  ptr +=".text{font-weight: 600;font-size: 19px;width: 200px;}";
  ptr +=".reading{font-weight: 300;font-size: 50px;padding-right: 25px;}";
  ptr +=".temperature .reading{color: #F29C1F;}";
  ptr +=".Tc .reading{color: #3B97D3;}";
  ptr +=".superscript{font-size: 17px;font-weight: 600;position: absolute;top: 10px;}";
  ptr +=".data{padding: 10px;}";
  ptr +=".container{display: table;margin: 0 auto;}";
  ptr +=".icon{width:65px}";
  ptr +="</style>";
  ptr +="<script>\n";
  ptr +="setInterval(loadDoc,1000);\n";
  ptr +="function loadDoc() {\n";
  ptr +="var xhttp = new XMLHttpRequest();\n";
  ptr +="xhttp.onreadystatechange = function() {\n";
  ptr +="if (this.readyState == 4 && this.status == 200) {\n";
  ptr +="document.body.innerHTML =this.responseText}\n";
  ptr +="};\n";
  ptr +="xhttp.open(\"GET\", \"/\", true);\n";
  ptr +="xhttp.send();\n";
  ptr +="}\n";
  ptr +="</script>\n";
  ptr +="</head>";
  ptr +="<body>";
  ptr +="<h1>BBQ Sensor</h1>";
  ptr +="<div class='container'>";
  ptr +="<div class='data temperature'>";
  ptr +="<div class='side-by-side icon'>";
  ptr +="<svg enable-background='new 0 0 19.438 54.003'height=54.003px id=Layer_1 version=1.1 viewBox='0 0 19.438 54.003'width=19.438px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><g><path d='M11.976,8.82v-2h4.084V6.063C16.06,2.715,13.345,0,9.996,0H9.313C5.965,0,3.252,2.715,3.252,6.063v30.982";
  ptr +="C1.261,38.825,0,41.403,0,44.286c0,5.367,4.351,9.718,9.719,9.718c5.368,0,9.719-4.351,9.719-9.718";
  ptr +="c0-2.943-1.312-5.574-3.378-7.355V18.436h-3.914v-2h3.914v-2.808h-4.084v-2h4.084V8.82H11.976z M15.302,44.833";
  ptr +="c0,3.083-2.5,5.583-5.583,5.583s-5.583-2.5-5.583-5.583c0-2.279,1.368-4.236,3.326-5.104V24.257C7.462,23.01,8.472,22,9.719,22";
  ptr +="s2.257,1.01,2.257,2.257V39.73C13.934,40.597,15.302,42.554,15.302,44.833z'fill=#F29C21 /></g></svg>";
  ptr +="</div>";
  ptr +="<div class='side-by-side text'>Meat Temperature</div>";
  ptr +="<div class='side-by-side reading'>";
  ptr +=(int)Tc;
  ptr +="<span class='superscript'>&deg;C</span></div>";
  ptr +="</div>";
  ptr +="</body>";
  ptr +="</html>";
  return ptr;
}
