# Wiz_Light_ESP_Library
An ESP library to control Wiz smart lights via WiFi. 

The Wiz lights can be controlled via a simple UDP protokoll. 

## Functionality
This library provides the following functionality for interacting with Wiz lights. 

`int Wiz::discoverLights(Wiz lights[], int maxNumLights)`  
Discovers lights available in the current WiFi network using a UDP broadcasts. 



## Dependencies
This library depends on [ArduinoJson](https://arduinojson.org/), the Arduino library, and the ESP Wifi and WifiUdp libraries. 


[Pywizlight](https://github.com/sbidy/pywizlight) has been a helpful reference during the development of this library. Big thanks to all the contributors. 
