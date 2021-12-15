#include <FS.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <WiFiClient.h>
#include <WiFi.h>
#include <WebServer.h>

// Internet config
const char* ssid = "";
const char* password = "";
IPAddress local_IP(192, 168, 1, 184);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);
 
String webPage = "";

//LED states

#define OFF 0
#define ON 1
#define BLINK 2

// LED groups
//1
int led1 = 13;
int led2 = 14;
int led3 = 26;
int ledChannel1 = 0;
byte stateLedGroup1 = 0;

//2
int led4 = 25;
int led5 = 33;
int led6 = 32;
int ledChannel2 = 1;
byte stateLedGroup2 = 0;

//3
int led7 = 23;
int led8 = 22;
int led9 = 5;
int ledChannel3 = 2;
byte stateLedGroup3 = 0;

//4
int led10 = 21;
int led11 = 19;
int led12 = 18; 
int ledChannel4 = 3;
byte stateLedGroup4 = 0;

//Presepio
int led13 = 4;
int ledChannel5 = 4;
byte stateLedGroup5 = 0;


int resolution = 8;
int freq = 5000;


// define directions for LED fade
#define UP 0
#define DOWN 1
 
// constants for min and max PWM
const int minPWM = 0;
const int maxPWM = 255;
 
// State Variable for Fade Direction
byte fadeDirection1 = UP;
byte fadeDirection2 = UP;
byte fadeDirection3 = UP;
byte fadeDirection4 = UP;
byte fadeDirection5 = UP;

// Set fade interval
int fadeInterval1 = 25;
int fadeInterval2 = 25;
int fadeInterval3 = 25;
int fadeInterval4 = 25;
int fadeInterval5 = 25;
 
// Global Fade Value
// but be bigger than byte and signed, for rollover
int fadeValue1 = 0;
int fadeValue2 = 0;
int fadeValue3 = 0;
int fadeValue4 = 0;
int fadeValue5 = 0;
 
// How smooth to fade?
byte fadeIncrement = 5;
 
// millis() timing Variable, just for fading
unsigned long previousFadeMillis1;
unsigned long previousFadeMillis2;
unsigned long previousFadeMillis3;
unsigned long previousFadeMillis4;
unsigned long previousFadeMillis5;
 
WebServer server(80);

void handleRoot() {
  server.send(200, "text/html", webPage);
}

void handleLedData() {
  String data = server.arg("plain");
  server.send(204,"");
  StaticJsonDocument<384> doc;
  deserializeJson(doc, data);
  String desiredStateGroup1 = doc["led1"];
  String desiredStateGroup2 = doc["led2"];
  String desiredStateGroup3 = doc["led3"];
  String desiredStateGroup4 = doc["led4"];
  String desiredStateGroup5 = doc["led5"];
  String desiredFadeInterval1 = doc["led1FadeInterval"];
  String desiredFadeInterval2 = doc["led2FadeInterval"];
  String desiredFadeInterval3 = doc["led3FadeInterval"];
  String desiredFadeInterval4 = doc["led4FadeInterval"];
  String desiredFadeInterval5 = doc["led5FadeInterval"];
  Serial.println("json");
  char buffer[2000];
  serializeJsonPretty(doc, buffer); 
  Serial.println(buffer);
   
  changeLedState(desiredStateGroup1, 1, desiredFadeInterval1);
  changeLedState(desiredStateGroup2, 2, desiredFadeInterval2);
  changeLedState(desiredStateGroup3, 3, desiredFadeInterval3);
  changeLedState(desiredStateGroup4, 4, desiredFadeInterval4);
  changeLedState(desiredStateGroup5, 5, desiredFadeInterval5);
}

void changeLedState(String state, int groupName, String desiredFadeInterval) {
  Serial.print("Group:");
  Serial.println(groupName);
  Serial.print("State:");
  Serial.println(state);
  Serial.print("Fade interval:");
  Serial.println(desiredFadeInterval);

  if(groupName==1){
    if(state == "blink") {
      stateLedGroup1=2;
      fadeInterval1=atoi(desiredFadeInterval.c_str());
    }
    else if(state =="off") {
      stateLedGroup1=0;
    }
    else if(state =="on") {
      stateLedGroup1=1;
    }
  } else if(groupName==2) {
    if(state == "blink") {
      stateLedGroup2=2;
      fadeInterval2=atoi(desiredFadeInterval.c_str());
    }
    else if(state =="off") {
      stateLedGroup2=0;
    }
    else if(state =="on") {
      stateLedGroup2=1;
    }
  } else if(groupName==3) {
    if(state == "blink") {
      stateLedGroup3=2;
      fadeInterval3=atoi(desiredFadeInterval.c_str());
    }
    else if(state =="off") {
      stateLedGroup3=0;
    }
    else if(state =="on") {
      stateLedGroup3=1;
    }
  } else if(groupName==4) {
    if(state == "blink") {
      stateLedGroup4=2;
      fadeInterval4=atoi(desiredFadeInterval.c_str());
    }
    else if(state =="off") {
      stateLedGroup4=0;
    }
    else if(state =="on") {
      stateLedGroup4=1;
    }
  } else if(groupName==5) {
    if(state == "blink") {
      stateLedGroup5=2;
      fadeInterval5=atoi(desiredFadeInterval.c_str());
    }
    else if(state =="off") {
      stateLedGroup5=0;
    }
    else if(state =="on") {
      stateLedGroup5=1;
    }
  }
}


void doTheFade(unsigned long thisMillis, int ledChannel, int fadeInterval, unsigned long *previousFadeMillis, byte *fadeDirection, int *fadeValue) {
  // is it time to update yet?
  // if not, nothing happens
  if (thisMillis - *previousFadeMillis >= fadeInterval) {
    // yup, it's time!
    if (*fadeDirection == UP) {
      *fadeValue = *fadeValue + fadeIncrement;  
      if (*fadeValue >= maxPWM) {
        // At max, limit and change direction
        *fadeValue = maxPWM;
        *fadeDirection = DOWN;
      }
    } else {
      //if we aren't going up, we're going down
      *fadeValue = *fadeValue - fadeIncrement;
      if (*fadeValue <= minPWM) {
        // At min, limit and change direction
        *fadeValue = minPWM;
        *fadeDirection = UP;
      }
    }
    // Only need to update when it changes
    // analogWrite(pwmLED, fadeValue); 
    ledcWrite(ledChannel, *fadeValue); 
 
    // reset millis for the next iteration (fade timer only)
    *previousFadeMillis = thisMillis;
  }
}

void readFile(void) {
  File rFile = SPIFFS.open("/index.html","r");
  Serial.println("Lendo arquivo HTML...");
  webPage = rFile.readString();
  rFile.close();  
}

void setupLedPins(){
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  
  pinMode(led4, OUTPUT);
  pinMode(led5, OUTPUT);
  pinMode(led6, OUTPUT);
  
  pinMode(led7, OUTPUT);
  pinMode(led8, OUTPUT);
  pinMode(led9, OUTPUT);
  
  pinMode(led10, OUTPUT);
  pinMode(led11, OUTPUT);
  pinMode(led12, OUTPUT);
  
  pinMode(led13, OUTPUT);

  ledcAttachPin(led1, ledChannel1);
  ledcAttachPin(led2, ledChannel1);
  ledcAttachPin(led3, ledChannel1);
  
  ledcAttachPin(led4, ledChannel2);
  ledcAttachPin(led5, ledChannel2);
  ledcAttachPin(led6, ledChannel2);
  
  ledcAttachPin(led7, ledChannel3);
  ledcAttachPin(led8, ledChannel3);
  ledcAttachPin(led9, ledChannel3);
  
  ledcAttachPin(led10, ledChannel4);
  ledcAttachPin(led11, ledChannel4);
  ledcAttachPin(led12, ledChannel4);

  ledcAttachPin(led13, ledChannel5);

  ledcSetup(ledChannel1, freq, resolution);
  ledcSetup(ledChannel2, freq, resolution);
  ledcSetup(ledChannel3, freq, resolution);
  ledcSetup(ledChannel4, freq, resolution);
  ledcSetup(ledChannel5, freq, resolution);
} 

void setup() {
  setupLedPins();
  
  Serial.begin(9600);
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  Serial.println("novo codigo1");
  if(SPIFFS.exists("/index.html"))
  {
    Serial.println("\n\nfile exists!");
    readFile();
  }
  else Serial.println("\n\nNo File :(");
  
  WiFi.config(local_IP, gateway, subnet);
  
  WiFi.begin(ssid, password);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/changeledstate", handleLedData);
   
  server.begin();

}
 
void loop() {
  server.handleClient();
  unsigned long currentMillis = millis();

  //Group 1
  if(stateLedGroup1==2) {
    doTheFade(currentMillis, ledChannel1, fadeInterval1, &previousFadeMillis1, &fadeDirection1, &fadeValue1);
  }
  else if(stateLedGroup1==0){
    ledcWrite(ledChannel1, 0);
  }
  else if(stateLedGroup1==1){
    ledcWrite(ledChannel1, 255);
  }

  //Group 2
  if(stateLedGroup2==2) {
    doTheFade(currentMillis, ledChannel2, fadeInterval2, &previousFadeMillis2, &fadeDirection2, &fadeValue2);
  }
  else if(stateLedGroup2==0){
    ledcWrite(ledChannel2, 0);
  }
  else if(stateLedGroup2==1){
    ledcWrite(ledChannel2, 255);
  }

  //Group 3
  if(stateLedGroup3==2) {
    doTheFade(currentMillis, ledChannel3, fadeInterval3, &previousFadeMillis3, &fadeDirection3, &fadeValue3);
  }
  else if(stateLedGroup3==0){
    ledcWrite(ledChannel3, 0);
  }
  else if(stateLedGroup3==1){
    ledcWrite(ledChannel3, 255);
  }

  //Group 4
  if(stateLedGroup4==2) {
    doTheFade(currentMillis, ledChannel4, fadeInterval4, &previousFadeMillis4, &fadeDirection4, &fadeValue4);
  }
  else if(stateLedGroup4==0){
    ledcWrite(ledChannel4, 0);
  }
  else if(stateLedGroup4==1){
    ledcWrite(ledChannel4, 255);
  }

    //Group 4
  if(stateLedGroup5==2) {
    doTheFade(currentMillis, ledChannel5, fadeInterval5, &previousFadeMillis5, &fadeDirection5, &fadeValue5);
  }
  else if(stateLedGroup5==0){
    ledcWrite(ledChannel5, 0);
  }
  else if(stateLedGroup5==1){
    ledcWrite(ledChannel5, 255);
  }

}
