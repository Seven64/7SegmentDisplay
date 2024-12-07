#include "Header.h";

#include <NeoPixelBus.h>
//#include<IoAbstraction.h>

const uint16_t PixelCount = 30; 
const uint8_t PixelPin = D4;  

long lastDoublePointChange = 0;



int defaultSaturation = 10;
int colorSaturation = defaultSaturation;
RgbColor myColor = RgbColor(0, 255, 0);
RgbColor green(colorSaturation, 0, 0);
RgbColor red(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation, colorSaturation, colorSaturation);
RgbColor black(0, 0, 0);

const int encoderAPin = D3;
const int encoderBPin = D4;

double sinuscounter = 0;
double sinusdelta = 0.0003;

int Punktdelay = 500;
int Timedelay = 50;
int Tempdelay = 3000;
int ledPositionDoublePoint = 14;
int lastDoublePointState = 0;
int lastTimeState = 0;
long LetzteTemperaturanzeige = 0;



const int maximumEncoderValue = 255;

//NeoPixelBus<NeoRgbwFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);
//NeoPixelBus<NeoRgbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);
NeoPixelBus<NeoRgbFeature, NeoEsp8266Dma800KbpsMethod> strip(PixelCount, PixelPin);



void helligkeit(int neueHelligkeit)
{
  Serial.println("helligkeit(" + String(neueHelligkeit) + ")");
  //int sensorwert_1 = 0; //Variable für den Sensorwert mit 0 als Startwert
  //sensorwert_1 = map(analogRead(eingang_1), 0, 1023, 0, 255); // / 4;
  //Serial.println( );
  //Serial.println(sensorwert_1);
  //Serial.println( );
  //colorSaturation = sensorwert_1;
  colorSaturation = neueHelligkeit;

  myColor = RgbColor(colorSaturation, colorSaturation, colorSaturation);
  green = RgbColor(colorSaturation, 0, 0);
  red = RgbColor(0, colorSaturation, 0);
  blue = RgbColor(0, 0, colorSaturation);
  white = RgbColor(colorSaturation, colorSaturation, colorSaturation);

  //stripBrightness.SetBrightness(colorSaturation);
  //stripBrightness.Show();
}

//
//HslColor hslRed(red);
//HslColor hslGreen(green);
//HslColor hslBlue(blue);
//HslColor hslWhite(white);
//HslColor hslBlack(black);


int calculateFirstLedPosition(int p)
{
  if (p > 1) return 7 * p + 2;
  else return 7 * p;
}


int lastGreen = 0;
int lastRed = 0;
int lastBlue = 0;
RgbColor calculateColor(int LED_POS)
{
  /* -R-G-B- = GRB*/
  if (getNightMode()  == 1)
  {
    double m = sinuscounter;
    sinuscounter = sinuscounter + sinusdelta;
    int g = ((   sin(   ( m + LED_POS * sinusdelta                  ) / 1.0 ) + 1) / 2.0 * colorSaturation);
    int r = ((   sin(   ( m + LED_POS * sinusdelta + 2 * PI / 3.0   ) / 2.0 ) + 1) / 2.0 * colorSaturation);
    int b = ((   sin(   ( m + LED_POS * sinusdelta + 4 * PI / 3.0   ) / 3.0 ) + 1) / 2.0 * colorSaturation);
    return RgbColor(g, r, b);
  }

  else if (getNightMode()  == 0)
  {
    int green = (0);
    int red = (colorSaturation);
    int blue = (0);
    return RgbColor(green, red, blue);
  }

}

void Doppelpunkt() {

  if (millis() - lastDoublePointChange > Punktdelay) {
    lastDoublePointChange = millis();
    //    zeigeUhrzeit(aktuelleStunde, aktuelleMinute);
    //    zeigeUhrzeit(timeClient.getHours(), timeClient.getMinutes());
    lastDoublePointState = 1 - lastDoublePointState;
    digitalWrite(LED_BUILTIN,lastDoublePointState);

    if (lastDoublePointState == 0)
    {
      strip.SetPixelColor(ledPositionDoublePoint, black);
      strip.SetPixelColor(ledPositionDoublePoint + 1, black);
      // quick hack um die Farbwechselgeschwindigkeit auch bei ausgeschaltetem Doppelpunkt gleich zu halten
      calculateColor(ledPositionDoublePoint);
      calculateColor(ledPositionDoublePoint);
    }
    else
    {
      strip.SetPixelColor(ledPositionDoublePoint, calculateColor(ledPositionDoublePoint));
      strip.SetPixelColor(ledPositionDoublePoint + 1, calculateColor(ledPositionDoublePoint));
    }
    strip.Show();
  }
}

void DoublePointOff()
{
  lastDoublePointState = 0;
  strip.SetPixelColor(ledPositionDoublePoint, black);
  strip.SetPixelColor(ledPositionDoublePoint + 1, black);;
}

void Z0(int p) {
  int w = calculateFirstLedPosition(p);

  //  strip.SetPixelColor(0 + w, red);
  strip.SetPixelColor(0 + w, calculateColor(w));
  strip.SetPixelColor(1 + w, calculateColor(w));
  strip.SetPixelColor(2 + w, calculateColor(w));
  strip.SetPixelColor(3 + w, black);
  strip.SetPixelColor(4 + w, calculateColor(w));
  strip.SetPixelColor(5 + w, calculateColor(w));
  strip.SetPixelColor(6 + w, calculateColor(w));

  //strip.Show();
}

void Z1(int p) {
  int w = calculateFirstLedPosition(p);

  strip.SetPixelColor(0 + w, black);
  strip.SetPixelColor(1 + w, black);
  strip.SetPixelColor(2 + w, calculateColor(w));
  strip.SetPixelColor(3 + w, black);
  strip.SetPixelColor(4 + w, black);
  strip.SetPixelColor(5 + w, black);
  strip.SetPixelColor(6 + w, calculateColor(w));

  //strip.Show();
}

void Z2(int p) {
  int w = calculateFirstLedPosition(p);


  strip.SetPixelColor(0 + w, black);
  strip.SetPixelColor(1 + w, calculateColor(w));
  strip.SetPixelColor(2 + w, calculateColor(w));
  strip.SetPixelColor(3 + w, calculateColor(w));
  strip.SetPixelColor(4 + w, calculateColor(w));
  strip.SetPixelColor(5 + w, calculateColor(w));
  strip.SetPixelColor(6 + w, black);

  //strip.Show();
}

void Z3(int p) {
  int w = calculateFirstLedPosition(p);

  strip.SetPixelColor(0 + w, black);
  strip.SetPixelColor(1 + w, calculateColor(w));
  strip.SetPixelColor(2 + w, calculateColor(w));
  strip.SetPixelColor(3 + w, calculateColor(w));
  strip.SetPixelColor(4 + w, black);
  strip.SetPixelColor(5 + w, calculateColor(w));
  strip.SetPixelColor(6 + w, calculateColor(w));

  //strip.Show();
}

void Z4(int p) {
  int w = calculateFirstLedPosition(p);

  strip.SetPixelColor(0 + w, calculateColor(w));
  strip.SetPixelColor(1 + w, black);
  strip.SetPixelColor(2 + w, calculateColor(w));
  strip.SetPixelColor(3 + w, calculateColor(w));
  strip.SetPixelColor(4 + w, black);
  strip.SetPixelColor(5 + w, black);
  strip.SetPixelColor(6 + w, calculateColor(w));

  //strip.Show();
}

void Z5(int p) {
  int w = calculateFirstLedPosition(p);

  strip.SetPixelColor(0 + w, calculateColor(w));
  strip.SetPixelColor(1 + w, calculateColor(w));
  strip.SetPixelColor(2 + w, black);
  strip.SetPixelColor(3 + w, calculateColor(w));
  strip.SetPixelColor(4 + w, black);
  strip.SetPixelColor(5 + w, calculateColor(w));
  strip.SetPixelColor(6 + w, calculateColor(w));

  //strip.Show();
}

void Z6(int p) {
  int w = calculateFirstLedPosition(p);

  strip.SetPixelColor(0 + w, calculateColor(w));
  strip.SetPixelColor(1 + w, calculateColor(w));
  strip.SetPixelColor(2 + w, black);
  strip.SetPixelColor(3 + w, calculateColor(w));
  strip.SetPixelColor(4 + w, calculateColor(w));
  strip.SetPixelColor(5 + w, calculateColor(w));
  strip.SetPixelColor(6 + w, calculateColor(w));

  //strip.Show();
}

void Z7(int p) {
  int w = calculateFirstLedPosition(p);

  strip.SetPixelColor(0 + w, black);
  strip.SetPixelColor(1 + w, calculateColor(w));
  strip.SetPixelColor(2 + w, calculateColor(w));
  strip.SetPixelColor(3 + w, black);
  strip.SetPixelColor(4 + w, black);
  strip.SetPixelColor(5 + w, black);
  strip.SetPixelColor(6 + w, calculateColor(w));

  //strip.Show();
}

void Z8(int p) {
  int w = calculateFirstLedPosition(p);

  strip.SetPixelColor(0 + w, calculateColor(w));
  strip.SetPixelColor(1 + w, calculateColor(w));
  strip.SetPixelColor(2 + w, calculateColor(w));
  strip.SetPixelColor(3 + w, calculateColor(w));
  strip.SetPixelColor(4 + w, calculateColor(w));
  strip.SetPixelColor(5 + w, calculateColor(w));
  strip.SetPixelColor(6 + w, calculateColor(w));

  //strip.Show();
}

void Z9(int p) {
  int w = calculateFirstLedPosition(p);

  strip.SetPixelColor(0 + w, calculateColor(w));
  strip.SetPixelColor(1 + w, calculateColor(w));
  strip.SetPixelColor(2 + w, calculateColor(w));
  strip.SetPixelColor(3 + w, calculateColor(w));
  strip.SetPixelColor(4 + w, black);
  strip.SetPixelColor(5 + w, calculateColor(w));
  strip.SetPixelColor(6 + w, calculateColor(w));

  //strip.Show();
}

void Grad(int p) {
  int w = calculateFirstLedPosition(p);

  strip.SetPixelColor(0 + w, calculateColor(w));
  strip.SetPixelColor(1 + w, calculateColor(w));
  strip.SetPixelColor(2 + w, calculateColor(w));
  strip.SetPixelColor(3 + w, calculateColor(w));
  strip.SetPixelColor(4 + w, black);
  strip.SetPixelColor(5 + w, black);
  strip.SetPixelColor(6 + w, black);

  //strip.Show();
}

void Celsius(int p) {
  int w = calculateFirstLedPosition(p);

  strip.SetPixelColor(0 + w, calculateColor(w));
  strip.SetPixelColor(1 + w, calculateColor(w));
  strip.SetPixelColor(2 + w, black);
  strip.SetPixelColor(3 + w, black);
  strip.SetPixelColor(4 + w, calculateColor(w));
  strip.SetPixelColor(5 + w, calculateColor(w));
  strip.SetPixelColor(6 + w, black);

  //strip.Show();
}


void zeigeZifferAnPosition(int ziffer, int p) {
  // Serial.print(p);
  // Serial.print(":");
  // Serial.println(ziffer);

  switch (ziffer)
  {
    case 0:
      Z0(p);
      break;
    case 1:
      Z1(p);
      break;
    case 2:
      Z2(p);
      break;
    case 3:
      Z3(p);
      break;
    case 4:
      Z4(p);
      break;
    case 5:
      Z5(p);
      break;
    case 6:
      Z6(p);
      break;
    case 7:
      Z7(p);
      break;
    case 8:
      Z8(p);
      break;
    case 9:
      Z9(p);
      break;
    case 10:
      Grad(p);
      break;
    case 11:
      Celsius(p);
      break;
  }

}

void zeigeUhrzeit(int hour, int minutes) {

  int h10 = hour / 10;
  int h1 = hour - h10 * 10;

  int m10 = minutes / 10;
  int m1 = minutes - m10 * 10;


  zeigeZifferAnPosition(h10, 0);
  zeigeZifferAnPosition(h1, 1);
  zeigeZifferAnPosition(m10, 2);
  zeigeZifferAnPosition(m1, 3);

  strip.Show();


}

void Temperatur() {

  if (millis() - LetzteTemperaturanzeige > Tempdelay)
  {
    float temp = getTemperatur();
    LetzteTemperaturanzeige = millis();
    //    Serial.println("Temperatur("+String(temp)+")");

    int t10 = temp / 10;
    int t1 = temp - t10 * 10;

    zeigeZifferAnPosition(t10, 0);
    zeigeZifferAnPosition(t1, 1);
    zeigeZifferAnPosition(10, 2); // Grad
    zeigeZifferAnPosition(11, 3); // Celsius

    DoublePointOff();

    strip.Show();
  }
}

void zeigeRestLernzeit(int Minutenwert, int Sekundenwert) {

  int m10 = Minutenwert / 10;
  int m1 = Minutenwert - m10 * 10;

  int s10 = Sekundenwert / 10;
  int s1 = Sekundenwert - s10 * 10;


  zeigeZifferAnPosition(m10, 0);
  zeigeZifferAnPosition(m1, 1);
  zeigeZifferAnPosition(s10, 2);
  zeigeZifferAnPosition(s1, 3);

  strip.Show();

}
