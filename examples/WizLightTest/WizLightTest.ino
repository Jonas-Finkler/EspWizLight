
#include <Arduino.h>
#include <Wiz.h>

// I assume that maximally 8 lights are in the network. Adjust this if you have more.
const int MAX_NUM_LIGHTS = 8;
const char* SSID = "SSID";
const char* PASSWORD =  "PASSWORD";

Wiz lights[MAX_NUM_LIGHTS];
int numLights;

void setup(){
    Serial.begin(115200);
    Serial.print("Connecting to WiFi ");
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(100);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    // discover all lights on the network
    numLights = Wiz::discoverLights(lights, MAX_NUM_LIGHTS);
    Serial.printf("Discovered %i lights.\n", numLights);

    for(int i=0; i<numLights; i++) {
      Serial.println(lights[i].getIP());
      // lights[i].setTemperature(Wiz::TEMPERATURE_MIN);
      lights[i].setColor(200, 50, 50, 150, 150);
      lights[i].setDimming(50);
      lights[i].setState(true);
      lights[i].pushConfig();
    } 
    delay(30000);
    for(int i=0; i<numLights; i++) {
      lights[i].setState(false);
      lights[i].pushConfig();
    } 
  // if (nLights <= 0){
  //   // dont continue
  //   while(true){
  //     Serial.println("Error: no lights found");
  //     delay(1000);
  //   }
    
  // }
  // for (int i=0; i<nLights; i++){
  //   lights[i].pullConfig();
  // }
  // light = lights[0];
}

int loopCounter = 0;
void loop(){
  // for(int i=0; i<numLights; i++){
  //   lights[i].setState(i==loopCounter%numLights);
  //   lights[i].pushConfig();
  // }

  delay(10000);
  loopCounter ++;
}
