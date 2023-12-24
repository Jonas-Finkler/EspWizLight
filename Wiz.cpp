#include "Wiz.h"

int Wiz::discoverLights(Wiz lights[], int maxNumLights) {
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

  udp.beginPacket(broadcastIP, Wiz::WIZ_PORT);
  serializeJson(command, udp);
  udp.endPacket();

  int iLight = 0;
  while (true) {
    StaticJsonDocument<JSON_SIZE> response;
    int responseState = awaitResponse(udp, response);
    if (responseState !=
        WizResult::SUCCESS) {  // did not get any reply todo: differenciate
                               // between errors and timeout
      Serial.println("no success");
      break;
    }

    lights[iLight] = Wiz(udp.remoteIP());
    lights[iLight].mac = response["result"]["mac"].as<String>();

    Serial.println("Discovered a light.");
    Serial.print("  Message: ");
    serializeJson(response, Serial);
    Serial.println("");
    Serial.print("  IP: ");
    Serial.println(lights[iLight].ip);
    Serial.print("  mac: ");
    Serial.println(lights[iLight].mac);

    iLight++;
    if (iLight >= maxNumLights) {
      break;  // exit loop
              // otherwise, wait for more packets from other lamps
              // todo: error handling
    }
  }

  udp.stop();
  return iLight;
}

WizResult Wiz::sendCommand(StaticJsonDocument<JSON_SIZE> command,
                           StaticJsonDocument<JSON_SIZE> &response) {
  WiFiUDP udp;
  // Already start listenting for response
  udp.begin(WIZ_PORT);
  udp.beginPacket(this->ip, Wiz::WIZ_PORT);
  serializeJson(command, udp);
  udp.endPacket();
  WizResult responseState = awaitResponse(udp, response);
  udp.stop();
  return responseState;
}

WizResult Wiz::awaitResponse(WiFiUDP &udp,
                             StaticJsonDocument<JSON_SIZE> &response) {
  int start = millis();
  while (start + Wiz::TIMEOUT > millis()) {
    int packetSize = udp.parsePacket();
    if (packetSize) {
      char packetBuffer[packetSize + 1];
      int len = udp.read(packetBuffer, packetSize);
      packetBuffer[len] = 0;  // terminate string
      // Serial.println(packetBuffer);

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
  Serial.println("timeout");
  return WizResult::TIMEOUT;
}

void Wiz::setColor(int r, int g, int b, int c, int w) {
  config.r = r;
  config.g = g;
  config.b = b;
  config.c = c;
  config.w = w;
  config.mode = RGBCW_MODE;
}

void Wiz::setTemperature(int temperature) {
  config.mode = TEMPERATURE_MODE;
  config.temperature = temperature;
}

void Wiz::setState(bool state) { config.state = state; }

bool Wiz::getState() { return config.state; }

void Wiz::setDimming(int dimming) {
  config.dimming = max(min(dimming, DIM_MAX), DIM_MIN);
}

int Wiz::getDimming() { return config.dimming; }

int Wiz::getTemperature() {
  if (config.mode == TEMPERATURE_MODE) {
    return config.temperature;
  } else {
    return -1;
  }
}

WizResult Wiz::pullConfig() {
  StaticJsonDocument<JSON_SIZE> command;
  StaticJsonDocument<JSON_SIZE> response;
  command["method"] = "getPilot";
  command.createNestedArray("params");  // add empty params:{}
  // serializeJson(command, Serial);
  // Serial.println("");

  WizResult sendResult = sendCommand(command, response);
  if (sendResult == WizResult::SUCCESS) {
    // Serial.print("Lamp state: ");
    // serializeJson(response, Serial);
    // Serial.println("");
    config.state = response["result"]["state"];
    config.sceneId = response["result"]["sceneId"];
    // some scenes (the wakeup one) do not contain a dim value
    // so we just set it to -1 so we will not send back 0 and cause an error the
    // next time
    if (response["result"].containsKey("dimming")) {
      config.dimming = response["result"]["dimming"];
    } else {
      config.dimming = -1;
    }
    if (config.sceneId == 0) {
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
  // todo: error handling
}

WizResult Wiz::pushConfig() {
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
      command["params"]["sceneId"] = config.sceneId;
      break;
  }

  WizResult sendResult = sendCommand(command, response);
  // Serial.print("Command: ");
  // serializeJson(command, Serial);
  // Serial.println("");
  // Serial.print("Response: ");
  // serializeJson(response, Serial);
  // Serial.println("");
  if (sendResult != WizResult::SUCCESS) {
    return sendResult;
  }
  if (response.containsKey("error")) {
    return WizResult::LIGHT_ERROR;
    // const char* errorMessage = response["error"]["message"];
    // int errorCode = response["error"]["code"];
    // todo: catch the parameter error here
  }

  if (response["result"].containsKey("success")) {
    if (response["result"]["success"]) {
      return WizResult::SUCCESS;
    }
  }
  return WizResult::ERROR;
}

LightConfig Wiz::getConfig() { return config; }

IPAddress Wiz::getIP() { return ip; }