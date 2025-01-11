#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

WiFiUDP ntpUDP;
//NTPClient timeClient(ntpUDP, "192.168.0.97");
NTPClient timeClient(ntpUDP, "192.168.0.97", 3600, 600000);

bool schonmalZeitAbgeholt = 0;
long lastTimezoneUpdate = 0;
double lastNtpUpdate = 0;


String weekdays[8] = {"?So?", "Mo", "Di", "Mi", "Do", "Fr", "Sa", "So"}; // 1-7


char ssid[] = "WLAN";
char pass[] = "PASSWORT";


int aktuelleStunde = 0;
int aktuelleMinute = 0;
int aktuelleSekunde = 0;

int alarmHour = 5;      // Weckseit default bei 5:45h
int alarmMinute = 45;
int alarmSeconds = 60;  // 60 Sekunden lang wecken

//unsigned int localPort = 2390;

//IPAddress timeServer(192, 168, 0, 80);
IPAddress timeServer(192, 168, 0, 97);   




void setTimezoneOffset()
{
  if (millis() - lastTimezoneUpdate < 0
      || lastTimezoneUpdate == 0
      || lastNtpUpdate == 0
      || (millis() - lastTimezoneUpdate > 1800000 && timeClient.getHours() > 1 && timeClient.getHours() < 4))
  {
    WiFiClient client;
    const char* timezonedb = "api.timezonedb.com";
    if (client.connect(timezonedb, 80))
    {
      Serial.println("setTimezoneOffset(): Getting timezone offset from api.timezonedb.com...");

      String url = "/v2.1/get-time-zone?key=X32J2DIFS62R&format=xml&by=zone&zone=Europe/Berlin";

      // This will send the request to the server
      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: api.timezonedb.com\r\n" +
                   "Connection: close\r\n\r\n");

      // Read all the lines of the reply from server and print them to Serial
      //    Serial.println("Respond:");
      //    Serial.println(client.connected());
      //    Serial.println(client.available());
      while (!client.available())
      {
        Serial.print(".");
        delay(10);
      }
      Serial.println();

      String result = "";
      while (client.available()) {
        String line = client.readStringUntil('\n');
        //      Serial.println(line);
        if (line.indexOf("gmtOffset") > 0)
        {
          Serial.println("Timezone offset retrieved: result");
          result = line.substring(line.indexOf("<gmtOffset>") + 11, line.indexOf("</gmtOffset"));
          //        Serial.println(result);
        }
      }
      client.stop();
      if (result == "3600")
      {
        Serial.println("setTimezoneOffset(): Setting timezone offset to GMT+1 (winter time)");
        timeClient.setTimeOffset(3600);
        lastTimezoneUpdate = millis();
      }
      else if (result == "7200")
      {
        Serial.println("setTimezoneOffset(): Setting timezone offset to GMT+2 (summer time)");
        timeClient.setTimeOffset(7200);
        lastTimezoneUpdate = millis();
      }
      else
      {
        Serial.println("setTimezoneOffset(): Timezone offset not valid: " + result);
        Serial.println("setTimezoneOffset(): Given timezone offset not changed.");
        lastTimezoneUpdate = 0;
      }
    }
    else
    {
      Serial.println("setTimezoneOffset(): Could not retrieve timezone offset from api.timezonedb.com");
      Serial.println("setTimezoneOffset(): Given timezone offset not changed.");
      lastTimezoneUpdate = 0;
    }
  }
}

bool updateNtp()
{
  // nach 10 Minuten oder bei Überlauf von millis()
  if (lastNtpUpdate == 0 || millis() - lastNtpUpdate > 600000 || millis() - lastNtpUpdate < 0 ) // alle 1 Minuten oder wenn vermutlich die Zeit noch nicht synchronisiert wurde
  {
    setTimezoneOffset();
    timeClient.update();
    lastNtpUpdate = millis();

    return true;
  }
  return false;
}


void einmal() {
  Serial.print("Starting...");
  WiFi.begin(ssid, pass);

  pinMode(LED_BUILTIN,OUTPUT);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    Doppelpunkt();
    delay(500);
  }
  digitalWrite(LED_BUILTIN, 1);
  Serial.println();

  Serial.println();

  //  udp.begin(localPort);
  timeClient.begin();
  timeClient.setTimeOffset(36000);
  bool ntpSuccessful = updateNtp();
  if (ntpSuccessful)
  {
    Serial.println("\n" + weekdays[timeClient.getDay()] + " " + timeClient.getFormattedTime());
    Serial.println("Alarm: " + String(alarmHour) + ":" + String(alarmMinute));
  }
}

void zeitAbholen()
{
  bool ntpSuccessful = updateNtp();
  if (ntpSuccessful) Serial.println("\nzeitAbholen(): " + weekdays[timeClient.getDay()] + " " + timeClient.getFormattedTime());
}

//bool   schonmalZeitAbgeholt = 0;
long zeitpunkt1 = 0;
