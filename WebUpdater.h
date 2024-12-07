#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
//#include "detail/RequestHandler.h"
#include <functional>

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;



const char* host = "segmentclock";

const char* indexHTML = "<html><body><a href='/led1/toggle'>LED1 toggle</a></body></html>";



boolean mute = 0;
boolean httpHandleLed1toggle()
{

  Serial.println("serving request /led1/toggle ...");

  pinMode(LED_BUILTIN, OUTPUT);

  mute = 1 - mute;
  if (mute) helligkeit(10);
  else helligkeit(defaultSaturation);

  digitalWrite(LED_BUILTIN, 1 - digitalRead(LED_BUILTIN));

  //  Serial.println(curr_uri);
  httpServer.sendHeader("Connection", "close");
  httpServer.sendHeader("Access-Control-Allow-Origin", "*");
  httpServer.send(200, "text/html", indexHTML);
  return true;
}

boolean httpHandleSlider()
{
  // http://segmentclock.fritz.box/brightness/slider?slidervalue=50

  Serial.println("serving request /brightness/slider ...");

  String brightness=httpServer.arg("slidervalue");
  int i_brightness = brightness.toInt();
  helligkeit(i_brightness);

  //  Serial.println(curr_uri);
  httpServer.sendHeader("Connection", "close");
  httpServer.sendHeader("Access-Control-Allow-Origin", "*");
  httpServer.send(200, "text/html", indexHTML);
  return true;
}

void initializeWeb()
{
  Serial.println("Initializing own http request handler...");
  Serial.println(" WebSite is now online: http://" + WiFi.localIP().toString());
  httpServer.on("/led1/toggle", HTTP_GET, httpHandleLed1toggle);
  httpServer.on("/brightness/slider", HTTP_GET, httpHandleSlider);
}

void setup_Webupdater()
{
  MDNS.begin(host);
  httpUpdater.setup(&httpServer);
  httpServer.begin();

  initializeWeb();

  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", host);
}


void handleWeb()
{
  httpServer.handleClient();
}
