#ifndef WizLight_h
#define WizLight_h
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>

#include "WizScenes.h"

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#else
// go for WiFi.h as default
#include <WiFi.h>
#endif

enum LightMode {
  TEMPERATURE_MODE,  // White light of certain temperature
  RGBCW_MODE,        // User defined color
  SCENE_MODE,        // Wiz scene
  OTHER_MODE         // Mode is not specified or has not been pulled yet
};

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
  WizScene scene = NO_SCENE;  // 0 means no scene, for other scenes see:
  // https://github.com/sbidy/pywizlight/blob/master/pywizlight/scenes.py
};

class WizLight {
 private:
  static const int JSON_SIZE = 512;  // memory for json objects
  static const int WIZ_PORT = 38899;
  static const unsigned int TIMEOUT = 200;
  // todo: make private?
  IPAddress ip;
  String mac;
  // const char* mac;

  /**
   * Sends a command and waits for the response.
   * @param[in] command The command to be sent.
   * @param[out] response The response from the light.
   * @return Success or error
   */
  WizResult sendCommand(StaticJsonDocument<JSON_SIZE> command,
                        StaticJsonDocument<JSON_SIZE> &response);

  /**
   * Wait for a response from the light.
   * @param[in] udp The WifiUPD object on which the request was sent.
   * @param[out] response The json response from the light.
   * @return Success or error
   */
  static WizResult awaitResponse(WiFiUDP &udp,
                                 StaticJsonDocument<JSON_SIZE> &response);

  LightConfig config;

 public:
  WizLight(){};
  WizLight(IPAddress _ip) : ip{_ip} {};

  // todo: these should be read from the bulb
  static const int DIM_MIN = 10;
  static const int DIM_MAX = 100;
  static const int TEMPERATURE_MIN = 2200;
  static const int TEMPERATURE_MAX = 6500;

  // todo: implement constructor from IP

  /**
   * Discovers light on the local network using a UDP broadcast.
   * @param[out] lights Array of size maxNumLights.
   * @param[in] maxNumLights Maximum number of lights to be discovered.
   * @param[in] numTries How many broadcasts will be sent. Values >1 improve
   * discovery rate. Default=5
   * @return the number of lights discovered.
   */
  static int discoverLights(WizLight *lights, int maxNumLights,
                            int numTries = 5);

  /**
   * Requests the configuration from the light and waits for a response.
   * @return Whether the operation was success.
   */
  WizResult pullConfig();

  /**
   * Sends the configutation to the light and waits for a response.
   * @return Whether the operation was success.
   */
  WizResult pushConfig();

  /**
   * Set a user defined color.
   * @param[in] r Power of the red leds (0-255).
   * @param[in] g Power of the green leds (0-255).
   * @param[in] b Power of the blue leds (0-255).
   * @param[in] c Power of the cold white leds (0-255).
   * @param[in] w Power of the warm white leds (0-255).
   */
  void setColor(int r, int g, int b, int c = 0, int w = 0);

  /**
   * Set the light to a white with a specific temperature.
   * @param[in] temp Temperature of the white light (WizLight::TEMPERATURE_MIN -
   *                 WizLight::TEMPERATURE_MAX).
   */
  void setTemperature(int temp);

  /**
   * Turn the light on or off.
   * @param[in] state Wheteher the light is on.
   */
  void setState(bool state);

  /**
   * Returns whether the light was on during the last pullConfig operation.
   * @return State of the light.
   */
  bool getState();

  /**
   * Set the dimming of the light.
   * @param[in] dimming Dimming value (WizLight:DIM_MIN - WizLight::DIM_MAX).
   */
  void setDimming(int dimming);

  /**
   * Returns the dimming of the light during the last pullConfig operation.
   * @return Dimming of the light.
   */
  int getDimming();

  /**
   * Set a specific scene
   * @param[in] scene The scene to be activated.
   */
  void setScene(WizScene scene);

  /**
   * Returns the scene of the light during the last pullConfig operation.
   * @return Scene of the light.
   */
  WizScene getScene();

  /**
   * Returns the color temperature of the light during the last pullConfig
   * operation.
   * @return Color temperature, or 0 in case the lamp is in a different mode
   *         than TEMPERATURE_MODE.
   */
  int getTemperature();

  /**
   * Returns the most recent configuration of the light.
   * @return The config obtained from the last pullConfig operation.
   */
  LightConfig getConfig();

  /**
   * Sets the config.
   */
  void setConfig(LightConfig _config);

  /**
   * Returns the IP address of the light.
   * @return the IP adress.
   */
  IPAddress getIP();

  /**
   * Returns the MAC adress of the light.
   * @return the MAC adress.
   */
  String getMac();

  // todo: Implement methods for requesting these configurations
  // {"method":"getModelConfig","env":"pro","result":{"ps":2,"pwmFreq":1000,"pwmRes":11,"pwmRange":[0,100],"wcr":60,"nowc":1,"cctRange":[2200,2700,6500,6500],"renderFactor":[200,255,150,255,0,0,40,0,0,0],"noSmartConf":0,"drvIface":0}}
  // {"method":"getUserConfig","env":"pro","result":{"fadeIn":500,"fadeOut":500,"dftDim":100,"opMode":0,"po":false,"minDimming":0,"tapSensor":1}}
};

#endif