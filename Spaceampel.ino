#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* SSID = "vspace.one";
const char* PSK = "12345678";
const char* MQTT_BROKER = "mqtt.vspace";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

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
}

void setup_wifi() {
  delay(10);
  //    Serial.println();
  //    Serial.print("Connecting to ");
  //    Serial.println(SSID);
  digitalWrite(0, HIGH);
  digitalWrite(2, HIGH);
  digitalWrite(1, LOW);

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

  if (strcmp(msg, "{\"status\":\"ok\",\"data\":{\"open\":true}}") == 0) {
    digitalWrite(0, HIGH);
    digitalWrite(2, LOW);
    //        Serial.println("Offen");
  }
  else if (strcmp(msg, "{\"status\":\"ok\",\"data\":{\"open\":false}}") == 0) {
    digitalWrite(0, LOW);
    digitalWrite(2, HIGH);
    //        Serial.println("Zu");
  }
  digitalWrite(1, HIGH);
  delay(5);
  digitalWrite(1, LOW);
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
}
