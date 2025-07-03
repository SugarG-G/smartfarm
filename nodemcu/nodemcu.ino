/*
    This sketch sends a string to a TCP server, and prints a one-line response.
    You must run a TCP server in your local network.
    For example, on Linux you can use this command: nc -v -l 3000
*/

/*
HIGH 릴레이 멈춤
LOW 릴레이 동작

#define relay_1 D1 왼 열
#define relay_2 D2 왼 닫
#define relay_3 D5 오 열
#define relay_4 D6 오 닫

*/

#include <ESP8266WiFi.h>
#include <Ticker.h>

// #define DEBUG_SocketEvent
// #define DEBUG_Connect_Server
// #define DEBUG_Connect_WiFi

#define relay_1 D1
#define relay_2 D2
#define relay_3 D5
#define relay_4 D6

#ifndef STASSID
#define STASSID "smart_farm"
#define STAPSK "123456789a!"
#endif

#define CMD_SIZE 50

#define LOGID "CHI_ARD"
#define PASSWD "PASSWD"

const char* ssid = STASSID;
const char* password = STAPSK;

const char* host = "chiyeong.asuscomm.com";
const uint16_t port = 5555;

volatile unsigned int cnt = 0;
bool timerIsrFlag = false;
bool relayStatusFlag = false;
char sendBuf[CMD_SIZE];


Ticker flipper;
WiFiClient client;

void socketEvent() {
  String recvStr;
  String idStr;
  String deviceStr;
  String statusStr;

  recvStr = client.readStringUntil('\n');

  #ifdef DEBUG_SocketEvent
    Serial.println(recvStr);
  #endif
  idStr = recvStr.substring(recvStr.indexOf("["), recvStr.indexOf("]") + 1);
  deviceStr = recvStr.substring(recvStr.indexOf("]") + 1, recvStr.indexOf("@"));
  statusStr = recvStr.substring(recvStr.indexOf("@") + 1, recvStr.length());
  #ifdef DEBUG_SocketEvent
    Serial.print("idStr : ");
    Serial.println(idStr);
    Serial.print("deviceStr : ");
    Serial.println(deviceStr);
    Serial.print("statusStr : ");
    Serial.println(statusStr);
  #endif
  if (deviceStr.startsWith(" New"))  // New Connected
  {
    Serial.write('\n');
    return;
  } else if (deviceStr.startsWith(" Alr"))  //Already logged
  {
    Serial.write('\n');
    client.stop();
    server_Connect();
    return;
  }
  else if (deviceStr == "LEFT") {
    if (statusStr == "OPEN"){
      digitalWrite(relay_1, LOW);
      digitalWrite(relay_2, HIGH);
    }
    else if (statusStr == "CLOSE") {
      digitalWrite(relay_1, HIGH);
      digitalWrite(relay_2, LOW);
    }
    else if (statusStr == "STOP") {
      digitalWrite(relay_1, HIGH);
      digitalWrite(relay_2, HIGH);
    }
    relayStatusFlag = true;
    recvStr.concat("\n");
    recvStr.toCharArray(sendBuf, CMD_SIZE);
  }
  else if(deviceStr == "RIGHT"){
    if (statusStr == "OPEN"){
      digitalWrite(relay_3, LOW);
      digitalWrite(relay_4, HIGH);
    }
    else if (statusStr == "CLOSE") {
      digitalWrite(relay_3, HIGH);
      digitalWrite(relay_4, LOW);
    }
    else if (statusStr == "STOP") {
      digitalWrite(relay_3, HIGH);
      digitalWrite(relay_4, HIGH);
    }
    relayStatusFlag = true;
    recvStr.concat("\n");
    recvStr.toCharArray(sendBuf, CMD_SIZE);
  }
  else if(deviceStr == "ALL"){
    if (statusStr == "OPEN"){
      digitalWrite(relay_1, LOW);
      digitalWrite(relay_2, HIGH);
      digitalWrite(relay_3, LOW);
      digitalWrite(relay_4, HIGH);
    }
    else if (statusStr == "CLOSE") {
      digitalWrite(relay_1, HIGH);
      digitalWrite(relay_2, LOW);
      digitalWrite(relay_3, HIGH);
      digitalWrite(relay_4, LOW);
    }
    else if (statusStr == "STOP") {
      digitalWrite(relay_1, HIGH);
      digitalWrite(relay_2, HIGH);
      digitalWrite(relay_3, HIGH);
      digitalWrite(relay_4, HIGH);
    }
    relayStatusFlag = true;
    recvStr.concat("\n");
    recvStr.toCharArray(sendBuf, CMD_SIZE);
  }

  client.write(sendBuf, strlen(sendBuf));
  #ifdef DEBUG_SocketEvent
    Serial.print(", send : ");
    Serial.print(sendBuf);
  #endif
  memset(sendBuf, 0, sizeof(sendBuf));
  client.flush();
}
void wifi_connect() {
  #ifdef DEBUG_Connect_WiFi
    Serial.print("Connecting to SSID : ");
    Serial.println(ssid);
  #endif
  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  // WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  #ifdef DEBUG_Connect_WiFi
    Serial.print("\nWiFi connected IP address: ");
    Serial.println(WiFi.localIP());
  #endif
}
void server_Connect() {
  #ifdef DEBUG_Connect_Server
    Serial.print("connecting to ");
    Serial.print(host);
    Serial.print(':');
    Serial.println(port);
  #endif
  if (client.connect(host, port)) {
    #ifdef DEBUG_Connect_Server
      Serial.println("Connected to server");
    #endif
    client.print(LOGID ":" PASSWD);
  } else {
    #ifdef DEBUG_Connect_Server
      Serial.println("server connection failure");
    #endif
  }
}
void pin_init() {
  pinMode(relay_1, OUTPUT);
  pinMode(relay_2, OUTPUT);
  pinMode(relay_3, OUTPUT);
  pinMode(relay_4, OUTPUT);

  digitalWrite(relay_1, HIGH);
  digitalWrite(relay_2, HIGH);
  digitalWrite(relay_3, HIGH);
  digitalWrite(relay_4, HIGH);
}
void flip() {
  timerIsrFlag = true;
  cnt++;
}
void setup() {
  Serial.begin(115200);

  // We start by connecting to a WiFi network
  wifi_connect();
  server_Connect();
  pin_init();
  flipper.attach(1, flip);
}
void loop() {

  if (client.available()) {
    socketEvent();
  }

  if (timerIsrFlag) {
    timerIsrFlag = false;

    if(cnt % 5)    {
      if (!client.connected()) {
        server_Connect();
      }
    }

    if(!(cnt % 600)){
      if(relayStatusFlag){
        digitalWrite(relay_1, HIGH);
        digitalWrite(relay_2, HIGH);
        digitalWrite(relay_3, HIGH);
        digitalWrite(relay_4, HIGH);
        // Serial.print("relay all off");
        relayStatusFlag = false;
      }
    }
  }
}
