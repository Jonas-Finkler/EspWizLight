#include "EspWizLight.h"

int WizLight::discoverLights(WizLight lights[], int maxNumLights,
                             int numTries) {
  // todo: Maybe use multiple requests here to improve success chances.
  // const char* msg =
  // "{\"method\":\"registration\",\"params\":{\"phoneMac\":\"AAAAAAAAAAAA\",\"register\":false,\"phoneIp\":\"1.2.3.4\",\"id\":\"1\"}}";
  StaticJsonDocument<JSON_SIZE> command;
  // command["method"] = "getPilot"; // the getPilot seems to not work when the
  // lights are off command.createNestedArray("params"); // add empty params:{}
  command["method"] = "registration";
  command["params"]["phoneMac"] = "AAAAAAAAAAAA";
  command["params"]["register"] = false;
  command["params"]["phoneIp"] = "1.2.3.4";
  // command["params"]["id"] = "1";
  WiFiUDP udp;
  // Already start listenting for response
  udp.begin(WIZ_PORT);
  IPAddress broadcastIP = WiFi.localIP();
  // The broadcast IP ends with 255
  broadcastIP[3] = 255;
  int numLights = 0;

  for (int iTry = 0; iTry < numTries; iTry++) {
    udp.beginPacket(broadcastIP, WizLight::WIZ_PORT);
    serializeJson(command, udp);
    udp.endPacket();

    while (true) {
      StaticJsonDocument<JSON_SIZE> response;
      int responseState = awaitResponse(udp, response);
      if (responseState != WizResult::SUCCESS) {
        break;
      }

      IPAddress lightIP = udp.remoteIP();
      bool lightAlreadyFound = false;
      for (int i = 0; i < numLights; i++) {
        if (lights[i].getIP() == lightIP) {
          lightAlreadyFound = true;
          break;
        }
      }
      if (!lightAlreadyFound) {
        lights[numLights] = WizLight(lightIP);
        lights[numLights].mac = response["result"]["mac"].as<String>();
        numLights++;
        if (numLights >= maxNumLights) {  // we are done here
          udp.stop();
          return numLights;
        }
      }
    }
  }

  udp.stop();
  return numLights;
}

WizResult WizLight::sendCommand(StaticJsonDocument<JSON_SIZE> command,
                                StaticJsonDocument<JSON_SIZE> &response) {
  WiFiUDP udp;
  // Already start listenting for response
  udp.begin(WIZ_PORT);
  udp.beginPacket(this->ip, WizLight::WIZ_PORT);
  serializeJson(command, udp);
  udp.endPacket();
  WizResult responseState = awaitResponse(udp, response);
  udp.stop();
  return responseState;
}

WizResult WizLight::awaitResponse(WiFiUDP &udp,
                                  StaticJsonDocument<JSON_SIZE> &response) {
  int start = millis();
  while (start + WizLight::TIMEOUT > millis()) {
    int packetSize = udp.parsePacket();
    if (packetSize) {
      char packetBuffer[packetSize + 1];
      int len = udp.read(packetBuffer, packetSize);
      packetBuffer[len] = 0;  // terminate string

      // we need this cast here
      // otherwise, the json lib will work with references to packetBuffer,
      // which will soon go out of scope
      DeserializationError error =
          deserializeJson(response, (const char *)packetBuffer);
      if (error) {
        // todo: Error handling
        Serial.println("deserialization error");
        return WizResult::DESERIALIZATION_ERROR;
      } else {
        return WizResult::SUCCESS;
      }
    }
  }
  // Serial.println("timeout");
  return WizResult::TIMEOUT;
}

void WizLight::setColor(int r, int g, int b, int c, int w) {
  config.r = r;
  config.g = g;
  config.b = b;
  config.c = c;
  config.w = w;
  config.mode = RGBCW_MODE;
}

void WizLight::setTemperature(int temperature) {
  config.mode = TEMPERATURE_MODE;
  config.temperature = temperature;
}

void WizLight::setState(bool state) { config.state = state; }

bool WizLight::getState() { return config.state; }

void WizLight::setDimming(int dimming) {
  config.dimming = max(min(dimming, DIM_MAX), DIM_MIN);
}

int WizLight::getDimming() { return config.dimming; }

void WizLight::setScene(WizScene scene) {
  config.mode = SCENE_MODE;
  config.scene = scene;
}

WizScene WizLight::getScene() { return config.scene; }

int WizLight::getTemperature() {
  if (config.mode == TEMPERATURE_MODE) {
    return config.temperature;
  } else {
    return -1;
  }
}

WizResult WizLight::pullConfig() {
  StaticJsonDocument<JSON_SIZE> command;
  StaticJsonDocument<JSON_SIZE> response;
  command["method"] = "getPilot";
  command.createNestedArray("params");  // add empty params:{}

  WizResult sendResult = sendCommand(command, response);
  if (sendResult == WizResult::SUCCESS) {
    config.state = response["result"]["state"];
    int sceneId = response["result"]["sceneId"];
    config.scene = (WizScene)sceneId;
    // some scenes (the wakeup one) do not contain a dim value
    // so we just set it to -1 so we will not send back 0 and cause an error the
    // next time
    if (response["result"].containsKey("dimming")) {
      config.dimming = response["result"]["dimming"];
    } else {
      config.dimming = -1;
    }
    if (config.scene == NO_SCENE) {
      if (response["result"].containsKey("temp")) {
        config.mode = TEMPERATURE_MODE;
        config.temperature = response["result"]["temp"];
      } else if (response["result"].containsKey("r")) {
        config.mode = RGBCW_MODE;
        config.r = response["result"]["r"];
        config.g = response["result"]["g"];
        config.b = response["result"]["b"];
        config.c = response["result"]["c"];
        config.w = response["result"]["w"];

      } else {
        config.mode = OTHER_MODE;
      }
    } else {
      config.mode = SCENE_MODE;
    }

    return WizResult::SUCCESS;
  } else {
    return sendResult;
  }
}

WizResult WizLight::pushConfig() {
  StaticJsonDocument<JSON_SIZE> command;
  StaticJsonDocument<JSON_SIZE> response;

  command["method"] = "setPilot";
  command["params"]["state"] = config.state;
  if (config.dimming >= 0) {
    command["params"]["dimming"] = config.dimming;
  }
  switch (config.mode) {
    case RGBCW_MODE:
      command["params"]["r"] = config.r;
      command["params"]["g"] = config.g;
      command["params"]["b"] = config.b;
      command["params"]["c"] = config.c;
      command["params"]["w"] = config.w;
      break;

    case TEMPERATURE_MODE:
      command["params"]["temp"] = config.temperature;
      break;

    case SCENE_MODE:
      command["params"]["sceneId"] = (int)config.scene;
      break;
  }

  WizResult sendResult = sendCommand(command, response);
  if (sendResult != WizResult::SUCCESS) {
    return sendResult;
  }
  if (response.containsKey("error")) {
    return WizResult::LIGHT_ERROR;
    // const char* errorMessage = response["error"]["message"];
    // int errorCode = response["error"]["code"];
    // todo: Maybe catch the parameter error here?
  }

  if (response["result"].containsKey("success")) {
    if (response["result"]["success"]) {
      return WizResult::SUCCESS;
    }
  }
  return WizResult::ERROR;
}

LightConfig WizLight::getConfig() { return config; }

IPAddress WizLight::getIP() { return ip; }
String WizLight::getMac() { return mac; }