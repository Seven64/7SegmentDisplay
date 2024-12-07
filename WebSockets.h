#include "Header.h";
#include <WebSocketsServer.h>
#include <Hash.h>

#include "Filesystem.h";





WebSocketsServer webSocket = WebSocketsServer(81);
//ESP8266WebServer server(80);

/*
  String WebPage ="<!DOCTYPE html><html><style>input[type=\"text\"]{width: 90%; height: 3vh;}input[type=\"button\"]{width: 9%; height: 3.6vh;}.rxd{height: 90vh;}textarea{width: 99%; height: 100%; resize: none;}</style><script>var Socket;function start(){Socket=new WebSocket('ws://' + window.location.hostname + ':81/'); Socket.onmessage=function(evt){document.getElementById(\"rxConsole\").value +=evt.data;}}function enterpressed(){Socket.send(document.getElementById(\"txbuff\").value); document.getElementById(\"txbuff\").value=\"\";}</script><body onload=\"javascript:start();\"> <div><input class=\"txd\" type=\"text\" id=\"txbuff\" onkeydown=\"if(event.keyCode==13) enterpressed();\"><input class=\"txd\" type=\"button\" onclick=\"enterpressed();\" value=\"Send\" > </div><br><div class=\"rxd\"> <textarea id=\"rxConsole\" readonly></textarea> </div></body></html>";

  void setup_WebSockets()
  {
  Serial.println("setup_WebSockets()");
  httpServer.on("/", []() {
    httpServer.send(200, "text/html", WebPage);
  });
  }
*/

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  String s_payload = "";
  String s_value = "";
  int value = 0;
  switch (type)
  {
    case WStype_CONNECTED:
      webSocket.sendTXT(num, "o" + String(colorSaturation > 0 ? 1 : 0));
      webSocket.sendTXT(num, "d" + String(colorSaturation));
      webSocket.sendTXT(num, "m" + String(getLoopMode()));

      break;
    case WStype_DISCONNECTED:
      Serial.println("webSocketEvent(): WStype_DISCONNECTED");
      break;
    case WStype_TEXT:
      s_payload = String((char*)payload);
      //payload[length] = '\0';
      Serial.println("webSocketEvent(): received payload '" + s_payload + "'");



      switch (s_payload[0])
      {
        case 'd':
          s_value = s_payload.substring(1);
          value = s_value.toInt();
          Serial.println("webSocketEvent(): Dimming to " + String(value));
          if (value > 255) value = 255;
          if (value < 0) value = 0;
          helligkeit(value);
          webSocket.sendTXT(num, "d" + String(colorSaturation));
          break;

        case 't':
          s_value = s_payload.substring(1);
          value = s_value.toInt();
          if (value < 0) value = 0;
          Serial.println("webSocketEvent(): Set time to " + String(value));
          setAusschaltzeit(value); // Ausschaltzeit in Minuten ab jetzt!
          break;

        case 'o':
          s_value = s_payload.substring(1);
          value = s_value.toInt();
          Serial.println("webSocketEvent(): Set time to " + String(value));
          helligkeit(value * 255); // Helligkeit auf 0/1 * 255 setzen
          webSocket.sendTXT(num, "o" + String(colorSaturation > 0 ? 1 : 0));
          webSocket.sendTXT(num, "d" + String(colorSaturation));
          break;

        case 's': // Statusabfrage
          webSocket.sendTXT(num, "o" + String(colorSaturation > 0 ? 1 : 0));
          webSocket.sendTXT(num, "d" + String(colorSaturation));
          webSocket.sendTXT(num, "m" + String(getLoopMode()));
          break;

        case 'T':
          setLoopMode(0);
          //loopTimeAndTemp();
          webSocket.sendTXT(num, "T");
          webSocket.sendTXT(num, "M" + String(getLoopMode()));
          break;

        case 'U':
          setLoopMode(1);
          //loopTime();
          webSocket.sendTXT(num, "U");
          webSocket.sendTXT(num, "M" + String(getLoopMode()));
          break;

        case 'L':
          s_value = s_payload.substring(1);
          value = s_value.toInt();
          // setLearnTime(value * 1000 * 60);
          setLearnTime(value);
          setLoopMode(2);
          //loopLernzeitTimer();
          webSocket.sendTXT(num, "L" + String(getLearnTime()));
          webSocket.sendTXT(num, "M" + String(getLoopMode()));
          break;

        case 'H':
          s_value = s_payload.substring(1);
          value = s_value.toInt();
          Serial.println("webSocketEvent(): Set AlarmHour to " + String(value));
          // setLearnTime(value * 1000 * 60);
          setAlarmHour(value);
          setLoopMode(3);
          webSocket.sendTXT(num, "H" + String(value));
          break;

        case 'M':
          s_value = s_payload.substring(1);
          value = s_value.toInt();
          Serial.println("webSocketEvent(): Set AlarmMinute to " + String(value));
          // setLearnTime(value * 1000 * 60);
          setAlarmMinute(value);
          setLoopMode(3);
          webSocket.sendTXT(num, "M" + String(value));
          break;

        case 'A':
          s_value = s_payload.substring(1);
          value = s_value.toInt();
          Serial.println("webSocketEvent(): Set AlarmTime to " + String(value));
          // setLearnTime(value * 1000 * 60);
          setAlarmHour(value / 100); // z.B. 545 /100 = 5 (Integer ohne Komma)
          setAlarmMinute(value - (value / 100) * 100); // z.B. 545 - (545 / 100) * 100 = 545 - 500 = 45
          setLoopMode(3);
          webSocket.sendTXT(num, "A" + String(value));
          break;

        case 'N':
          s_value = s_payload.substring(1);
          value = s_value.toInt();
          Serial.println("webSocketEvent(): Dimming to " + String(value));
          if (value > 255) value = 255;
          if (value < 0) value = 0;
          helligkeit(value);
          webSocket.sendTXT(num, "d" + String(colorSaturation));
          setNightMode(1);
          webSocket.sendTXT(num, "N" + String(getNightMode()));
          break;

        case 'O':
          s_value = s_payload.substring(1);
          value = s_value.toInt();
          setNightMode(0);
          webSocket.sendTXT(num, "O" + String(getNightMode()));
          break;

        default:
          Serial.println("webSocketEvent(): keinen bekannten Befehl gefunden");
          break;
      }
      break;
    default:
      Serial.println("webSocketEvent(): unbekannter Nachrichtentyp! Type=" + String(type));
      break;
  }
}

void start_WebSockets()
{
  Serial.println("start_WebSockets()");
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}


void loop_WebSockets() {
  webSocket.loop();
  if (Serial.available() > 0) {
    char c[] = {(char)Serial.read()};
    webSocket.broadcastTXT(c, sizeof(c));
  }
}
