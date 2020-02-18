#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include <Ticker.h>
#include <ArduinoOTA.h>

const char* SSID = "SSID";
const char* PSK = "PASSWORD";
const char* MQTT_BROKER = "BROKER_DNS";
#define UPDATEPW "UPDATE_PASSWORD"

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
//59c3dc1a8b797
int opened = -1;
int opened_old = -1;
bool tick_flag = 0;
int transmission_delay = 0;

Ticker ticker;
void tick() {
  tick_flag = true;
  if (transmission_delay > 0) {
    transmission_delay--;
  }
}

bool telegram(String Message = "[SpaceAmpel_Fehler: Kein Text an Senderoutine]", String Token = "59c3dc1a8b797") {
  int c;
  String Send = "";
  for (int i = 0; i < Message.length(); i++) {
    c = Message.charAt(i);
    if ( ('a' <= c && c <= 'z')
         || ('A' <= c && c <= 'Z')
         || ('0' <= c && c <= '9') ) {
      Send += char(c);
    } else {
      if (c <= 0x0F) {
        Send += "%0" + String(c, HEX);
      } else {
        Send += "%" + String(c, HEX);
      }
    }
  }

  HTTPClient http;
  http.begin("http://telegramiotbot.com/api/notify?token=" + Token + "&message=" + Send);
  int httpCode = http.GET();
  return httpCode != HTTP_CODE_OK;
}



void setup() {
  //********** CHANGE PIN FUNCTION  TO GPIO **********
  //GPIO 1 (TX) swap the pin to a GPIO.
  pinMode(1, FUNCTION_3);
  //GPIO 3 (RX) swap the pin to a GPIO.
  pinMode(3, FUNCTION_3);
  //**************************************************
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
  pinMode(2, OUTPUT);

  //    Serial.begin(115200);
  setup_wifi();
  client.setServer(MQTT_BROKER, 1883);
  client.setCallback(callback);

  
  
  
  ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname("SpaceAmpel");
#ifdef UPDATEPW
  ArduinoOTA.setPassword(UPDATEPW);
#endif

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    //Serial.println("Start updating " + type);
    //telegram("SpaceAmpel:\nStart updating " + type + " …");
  });
  ArduinoOTA.onEnd([]() {
    //Serial.println("\nEnd");
    telegram("SpaceAmpel: Firmwareupdate abgeschlossen!");
  });
  //ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
  //Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  //});
  ArduinoOTA.onError([](ota_error_t error) {
    //Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      //Serial.println("Auth Failed");
      telegram("SpaceAmpel:\nFOTA_ERROR: Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      //Serial.println("Begin Failed");
      telegram("SpaceAmpel:\nFOTA_ERROR: Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      //Serial.println("Connect Failed");
      telegram("SpaceAmpel:\nFOTA_ERROR: Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      //Serial.println("Receive Failed");
      telegram("SpaceAmpel:\nFOTA_ERROR: Receive Failed");
    } else if (error == OTA_END_ERROR) {
      //Serial.println("End Failed");
      telegram("SpaceAmpel:\nFOTA_ERROR: End Failed");
    }
    ESP.restart();
  });
  ArduinoOTA.begin();
  ticker.attach_ms(50, tick);
}

void setup_wifi() {
  delay(10);
  //    Serial.println();
  //    Serial.print("Connecting to ");
  //    Serial.println(SSID);
  digitalWrite(0, HIGH);
  digitalWrite(2, HIGH);
  digitalWrite(1, LOW);

  WiFi.hostname("SpaceAmpel");
  WiFi.begin(SSID, PSK);
  int i = 9;
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(0, LOW);
    digitalWrite(2, LOW);
    delay(250);
    digitalWrite(0, HIGH);
    digitalWrite(2, HIGH);
    if (i++ == 10) {
      i = 0;
      digitalWrite(1, HIGH);
      delay(5);
      digitalWrite(1, LOW);
    }
    delay(250);

    //       Serial.print(".");
  }
  digitalWrite(1, HIGH);
  delay(1);
  digitalWrite(1, LOW);
  delay(200);
  digitalWrite(1, HIGH);
  delay(1);
  digitalWrite(1, LOW);
  //    Serial.println("");
  //    Serial.println("WiFi connected");
  //    Serial.println("IP address: ");
  //    Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  //    Serial.print("Received message [");
  //    Serial.print(topic);
  //    Serial.print("] ");
  char msg[length + 1];
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    msg[i] = (char)payload[i];
  }
  //    Serial.println();

  msg[length] = '\0';
  //    Serial.println(msg);

  digitalWrite(1, HIGH);
  delay(5);
  digitalWrite(1, LOW);
  if (strcmp(msg, "{\"status\":\"ok\",\"data\":{\"open\":true}}") == 0) {
    digitalWrite(0, HIGH);
    digitalWrite(2, LOW);
    opened = 1;
//    telegram("vspace.one geöffnet");
    //        Serial.println("Offen");
  }
  else if (strcmp(msg, "{\"status\":\"ok\",\"data\":{\"open\":false}}") == 0) {
    digitalWrite(0, LOW);
    digitalWrite(2, HIGH);
    opened = 0;
//    telegram("vspace.one geschlossen");
    //        Serial.println("Zu");
  }
  transmission_delay = 200;
}

void reconnect() {
  while (!client.connected()) {
    //        Serial.println("Reconnecting MQTT...");
    if (!client.connect("ESP8266Client")) {
      //            Serial.print("failed, rc=");
      //            Serial.print(client.state());
      //            Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
  client.subscribe("vspace/one/state/open");
  //    Serial.println("MQTT Connected...");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  ArduinoOTA.handle();

  if (tick_flag){
    tick_flag = false;

    if (opened != opened_old && transmission_delay == 0){
      opened_old = opened;
      if (opened == 1){
        telegram("vspace.one geöffnet");
      }else{
        telegram("vspace.one geschlossen");      
      }
    }
  }
}
