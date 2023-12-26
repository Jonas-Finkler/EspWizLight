
#include <Arduino.h>
#include <EspWizLight.h>

// I assume that maximally 4 lights are in the network. Adjust this if you have
// more.
const int MAX_NUM_LIGHTS = 4;
const char* SSID = "SSID";
const char* PASSWORD = "PASSWORD";

WizLight lights[MAX_NUM_LIGHTS];
int numLights;

void setup() {
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
  numLights = WizLight::discoverLights(lights, MAX_NUM_LIGHTS);
  Serial.printf("Discovered %i lights.\n", numLights);
  for (int i = 0; i < numLights; i++) {
    Serial.printf("-- Light %i --\n", i);
    Serial.print("  IP: ");
    Serial.println(lights[i].getIP());
    lights[i].pullConfig();  // receive config from light
    LightConfig conf = lights[i].getConfig();
    Serial.printf("  State: %s\n", conf.state);
    Serial.printf("  Dimming: %i\n", conf.dimming);
    switch (conf.mode) {
      case RGBCW_MODE:
        Serial.println("  Mode: RGBCW");
        Serial.printf("  r: %i, g: %i, b: %i, c: %i, w: %i\n", conf.r, conf.g,
                      conf.b, conf.c, conf.w);
        break;
      case TEMPERATURE_MODE:
        Serial.println("  Mode: TEMPERATURE");
        Serial.printf("  Temperature: %i\n", conf.temperature);
        break;
      case SCENE_MODE:
        Serial.println("  Mode: SCENE");
        Serial.printf("  SceneId: %i\n", (int)conf.scene);
        break;

      default:
        Serial.println("  Mode: OTHER");
        break;
    }
  }

  // set all lights to a user defined color for 30 seconds
  for (int i = 0; i < numLights; i++) {
    lights[i].setColor(200, 50, 50, 150, 150);
    lights[i].setDimming(50);
    lights[i].setState(true);
    lights[i].pushConfig();
  }
  delay(30000);

  // set all lights to a 3000K white for 30 seconds
  for (int i = 0; i < numLights; i++) {
    lights[i].setTemperature(3000);
    lights[i].pushConfig();
  }
  delay(30000);

  // turn off all lights
  for (int i = 0; i < numLights; i++) {
    lights[i].setState(false);
    lights[i].pushConfig();
  }
}

void loop() {
  // do nothing here
}
