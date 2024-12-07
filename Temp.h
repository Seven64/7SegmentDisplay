#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS D2  // DS18B20 pin
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

float getTemperatur() {
  float temp;
  int maxtries = 5;
  do {
    DS18B20.requestTemperatures();
    temp = DS18B20.getTempCByIndex(0);
    Serial.print("!");
    maxtries--;
  } while (maxtries > 0 && (temp > 70 || temp < (0)));
  temp = temp - 3; // Korrektur, da die gemessene Temp immer ca. 3° zu hoch ist; Ursache unbekannt
  Serial.print("Temperature: ");
  Serial.println(temp);
  if (temp > 70 || temp < 0) temp = 99;
  return temp;
}
