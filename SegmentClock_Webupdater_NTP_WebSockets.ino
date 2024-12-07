/*
   ACHTUNG: SPIFFS muss einmalig beim Flashen aktiviert worden sein!!!
   Werkzeuge->FlashSize (3M SPIFFS)

   Danach per FileUpload die Files aus dem Unterordner "Files" hochladen
   http://192.168.0.254/upload
   edit.htm.gz
   http://192.168.0.254/edit
   index.htm
   http://192.168.0.254

*/


#include "Header.h";
#include "Temp.h";
#include "ZahlenLibrary.h";
#include "WebUpdater.h";
#include "WebSockets.h";
#include "Time.h";



int Pieps = D3;


int Buzzerdelay = 200;
int lastBuzzerState = 0;
int lastBuzzerChange = 0;


int taster = D5;
int tasterstatus = 0;

long Ausschaltzeit = 60000;// 1000 * 60 * 60 * 5; // automatisch ausschalten nach 5 Stunden
int Ausgeschaltet = 1; // default: Ausschalttimer deaktiviert


int Temp = A0;
int sensorwert = 0;
int temperaturwert = 0; //Unter der Variablen "temperatur" wird später der Temperaturwert abgespeichert.

int Wechselzeit = 10000;
int Wechsel = 1; // 0 = Temp, 1 = Time
long LetztesWechseln = 0;


int LearnTime = 30;
int LernTimeStartMillis = 0;


int Restlernsekunden = 0;
int Minutenrest = 0;
int Sekundenrest = 0;


int loopmode = 3; // 0 = Time and Temp / 1 = Time / 2 = Lernen / 3 = Time and Alarm
int nightmode = 0; // Nachmodus inaktiv

void setNightMode(int newmode)
{
  nightmode = newmode;
}

int getNightMode()
{
  return nightmode;
}

void setLoopMode(int newmode)
{
  loopmode = newmode;
}

int getLoopMode()
{
  return loopmode;
}

void setLearnTime(int newLearnTime)
{
  LernTimeStartMillis = millis();

  LearnTime = newLearnTime;
}

int getLearnTime()
{
  return LearnTime;
}

void setAusschaltzeit(int minutes)
{
  Ausschaltzeit = millis() + minutes * 1000 * 60;
  Ausgeschaltet = 0;
}


void setAlarmHour(int newHour)
{
  Serial.println("setAlarmHour(" + String(newHour) + ")");
  alarmHour = newHour;
}

int getAlarmHour()
{
  return alarmHour;
}

void setAlarmMinute(int newMinute)
{
  Serial.println("setAlarmMinute(" + String(newMinute) + ")");
  alarmMinute = newMinute;
}

int getMinute()
{
  return alarmMinute;
}


void setup() {

  pinMode(Pieps, OUTPUT);
  digitalWrite(Pieps, 0);


  pinMode(taster, INPUT_PULLUP);

  Serial.begin(115200);
  while (!Serial); // wait for serial attach

  Serial.println();
  Serial.println("Initializing...");
  Serial.flush();

  // Serial.println(D4);
  //Serial.println(LED_BUILTIN);

  // this resets all the neopixels to an off state
  strip.Begin();
  strip.Show();


  Serial.println();
  Serial.println("Running...");

  //zeigeUhrzeit(0, 10);


  //_________________________________________________________

  einmal();
  //  einmal2();
  setup_Filesystem();
  setup_Filesystem_Webhandlers();
  //  setup_WebSockets(); // wird nun vom Filesystem erledigt
  setup_Webupdater();
  start_WebSockets();

  LetztesWechseln = millis(); // ab jetzt zählt die Zeit des Wechselns zwischen Time und Temp
}


void loopTime()
{
  if (Ausschaltzeit < millis() ) {
    if (Ausgeschaltet == 0) {
      Serial.println("loop(): Ausschaltzeit erreicht: schalte nun aus.");
      Ausgeschaltet = 1;
      helligkeit(0);
    }
  }

  if (schonmalZeitAbgeholt == 0 || millis() >= zeitpunkt1 + 30000) // Zeit ist reif für neue Zeitabfrage
  {
    zeitAbholen();
    zeitpunkt1 = millis();
    schonmalZeitAbgeholt = 1;
  }

  if (schonmalZeitAbgeholt == 1) {
    Doppelpunkt();
    // lastDoublePointChange = millis();
    //    zeigeUhrzeit(aktuelleStunde, aktuelleMinute);
    //    zeigeUhrzeit(timeClient.getHours(), timeClient.getMinutes());
  }

  if (schonmalZeitAbgeholt == 1 && (millis() - lastTimeState > Timedelay)) {
    zeigeUhrzeit(timeClient.getHours(), timeClient.getMinutes());
    lastTimeState = millis();
  }
}


void loopTimeAndTemp()
{
  if (Ausschaltzeit < millis() ) {
    if (Ausgeschaltet == 0) {
      Serial.println("loop(): Ausschaltzeit erreicht: schalte nun aus.");
      Ausgeschaltet = 1;
      helligkeit(0);
    }
  }


  if (millis() - LetztesWechseln > Wechselzeit ) {
    Wechsel = 1 - Wechsel;
    LetztesWechseln = millis();
    Serial.println("loop(): Wechsel");
  }


  if (Wechsel == 0) { // aktuell soll Temp angezeigt werden
    Temperatur();
  }

  if (Wechsel == 1) {
    if (schonmalZeitAbgeholt == 0 || millis() >= zeitpunkt1 + 30000) // Zeit ist reif für neue Zeitabfrage
    {
      zeitAbholen();
      zeitpunkt1 = millis();
      schonmalZeitAbgeholt = 1;

    }

    //    if (schonmalZeitAbgeholt == 1 && (millis() - lastDoublePointChange > Punktdelay)) {
    if (schonmalZeitAbgeholt == 1) {
      Doppelpunkt();
      //      lastDoublePointChange = millis();
      //    zeigeUhrzeit(aktuelleStunde, aktuelleMinute);
      //    zeigeUhrzeit(timeClient.getHours(), timeClient.getMinutes());
    }


    if (schonmalZeitAbgeholt == 1 && (millis() - lastTimeState > Timedelay)) {
      zeigeUhrzeit(timeClient.getHours(), timeClient.getMinutes());
      lastTimeState = millis();
    }


  }

}


int lastMinutenrest = 0;
int lastSekundenrest = 0;
void loopLernzeitTimer()
{

  Restlernsekunden = LearnTime * 60 - (millis() - LernTimeStartMillis) / 1000;

  if (Restlernsekunden >= 0)
  {
    Minutenrest = Restlernsekunden / 60;
    Sekundenrest = Restlernsekunden - Minutenrest * 60;


    if ((millis() - lastTimeState > Timedelay) || lastMinutenrest != Minutenrest || lastSekundenrest != Sekundenrest)
    {
      zeigeRestLernzeit(Minutenrest, Sekundenrest);
    }

    lastMinutenrest = Minutenrest;
    lastSekundenrest = Sekundenrest;
  }
}

/*
  void Wecker () {

  if (timeClient.getHours() == 17 && timeClient.getMinutes() == 57) {

    Serial.println("Aufstehen");

  }

  }

*/

void loopAlarm()
{
  loopTime();

  if (timeClient.getDay() >= 1 || timeClient.getDay() <= 5) // nur an Wochentagen wecken
  {
    if (timeClient.getHours() == alarmHour && timeClient.getMinutes() == alarmMinute && timeClient.getSeconds() < alarmSeconds)
    {
      if (millis() - lastBuzzerChange > Buzzerdelay) // wenn nächste Buzzeränderung fällig
      {
        Serial.println("ALARM!!!" + String(timeClient.getFormattedTime()));
        lastBuzzerChange = millis();
        lastBuzzerState = 1 - lastBuzzerState; // Buzzer an/aus wechseln
      }
      if (lastBuzzerState == 0)
      {
        digitalWrite(LED_BUILTIN,lastBuzzerState);
        noTone(Pieps);
      }
      else
      {
        tone(Pieps, 392);
        digitalWrite(LED_BUILTIN,lastBuzzerState);
      }
    }
    else
    {
      noTone(Pieps);
//      digitalWrite(LED_BUILTIN,0);
    }
  }


}





void loop() {
  switch (loopmode) {
    case 0: loopTimeAndTemp();
      break;

    case 1: loopTime();
      break;

    case 2: loopLernzeitTimer();
      break;

    case 3: loopAlarm();
      break;

  }

  handleWeb();
  loop_WebSockets();


}
