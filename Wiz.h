#ifndef Wiz_h
#define Wiz_h
#include <ArduinoJson.h>
#include <WiFiUdp.h>

#include "Arduino.h"
#include "WiFi.h"

// todo: consistent language: lamp/light

enum LightMode { TEMPERATURE_MODE, RGBCW_MODE, SCENE_MODE, OTHER_MODE };

enum WizResult {
  SUCCESS = 0,
  ERROR,
  TIMEOUT,
  DESERIALIZATION_ERROR,
  LIGHT_ERROR  // When the light sends back an error (e.g. wrong parameters)
};

struct LightConfig {
  LightMode mode = OTHER_MODE;
  bool state = false;
  int r = 0;
  int g = 0;
  int b = 0;
  int c = 0;
  int w = 0;
  int temperature = 0;
  int dimming = 0;
  int sceneId = 0;  // 0 means no scene, for other scenes see:
  // https://github.com/sbidy/pywizlight/blob/master/pywizlight/scenes.py
};

class Wiz {
 private:
  // todo: use #define here?
  static const int JSON_SIZE = 512;  // memory for json objects
  static const int WIZ_PORT = 38899;
  static const int TIMEOUT = 2000;
  IPAddress ip;
  String mac;  // todo: use char[12] here
  // const char* mac;

  // Sends a command and waits for the response. Returns 0 on success.
  WizResult sendCommand(StaticJsonDocument<JSON_SIZE> command,
                        StaticJsonDocument<JSON_SIZE> &response);

  // Waits for a udp response from the light.
  static WizResult awaitResponse(WiFiUDP &udp,
                                 StaticJsonDocument<JSON_SIZE> &response);

  LightConfig config;

 public:
  Wiz(){};
  Wiz(IPAddress _ip) : ip{_ip} {};
  // todo: these should be read from the bulb
  static const int DIM_MIN = 10;
  static const int DIM_MAX = 100;
  static const int TEMPERATURE_MIN = 2200;
  static const int TEMPERATURE_MAX = 6500;

  // todo: implement constructor from IP

  // Uses a broadcast message to discover all lights in the network.
  // Returns the number of lights found.
  static int discoverLights(Wiz *lights, int maxNumLights);

  // Pulls the state of the light into the Wiz object
  // Returns 0 on success
  WizResult pullConfig();
  // Send configuration to the light todo: add option to not wait for response
  WizResult pushConfig();

  // Set the color of the light (c and w are the cold and warm white leds)
  void setColor(int r, int g, int b, int c = 0,
                int w = 0);  // todo: use bytes here
  // Set the color temperature of the light
  void setTemperature(int temp);
  // Turn the light on or off
  void setState(bool state);
  // Returns wether the lamp is on or off
  bool getState();
  // set the dimming of the light using a value from DIM_MIN to DIM_MAX
  void setDimming(int dimming);
  // returns the current dimming of the light
  int getDimming();
  // returns the current color temperature of the light or 0 in case an rgb
  // value or a different scene is currently used
  int getTemperature();
  // returns the state of the light (remember to call pullConfig before this)
  LightConfig getConfig();

  IPAddress getIP();

  // todo:
  // {"method":"getModelConfig","env":"pro","result":{"ps":2,"pwmFreq":1000,"pwmRes":11,"pwmRange":[0,100],"wcr":60,"nowc":1,"cctRange":[2200,2700,6500,6500],"renderFactor":[200,255,150,255,0,0,40,0,0,0],"noSmartConf":0,"drvIface":0}}
  // {"method":"getUserConfig","env":"pro","result":{"fadeIn":500,"fadeOut":500,"dftDim":100,"opMode":0,"po":false,"minDimming":0,"tapSensor":1}}
};

#endif

// getModelConfig
// getPilot