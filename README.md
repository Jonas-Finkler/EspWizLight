# Wiz_Light_ESP_Library

This library allows to controll Wiz smart lights with ESPs via UDP commands. 


## Usage

Start by creating an instance of your light.
```C++
Wiz light = Wiz(IpAddress(192, 168, 0, 30));
```
You can also automatically discover lights on the local network. 
```C++
const int maxNumLights = 3;
Wiz lights[maxNumLights];
int numLights = discoverLights(lights, maxNumLights);
```

You can set the light to a specific color. 
```C++
light.setColor(255, 0, 0); // set the light to red
light.setState(true); // turns the light on
light.setDimming(50); // dim the light to half the maximum brightness
light.pushConfig(); // send the configuration to the light
```
Always remember to use `pushConfig` to send your configuration to the light. 
In addition to the red, green, and blue leds, you can also control the cold and warm white leds individually.
```C++
light.setColor(0, 0, 0, 255, 0); // set the cold white leds to maximum power
light.setState(true); // turns the light on
light.setDimming(50); // dim the light to half the maximum brightness
light.pushConfig(); // send the configuration to the light
```

You can also set the light to a white light with a specific temperature. 
```C++
light.Temperature(3000); // set the light 3000K white
light.setState(true); // turns the light on
light.setDimming(50); // dim the light to half the maximum brightness
light.pushConfig(); // send the configuration to the light
```
The range of the tempereture is limited. 
You can find these values in `Wiz::TEMPERATURE_MIN` and `Wiz::TEMPERATURE_MAX`.

You can also obtain the current configuration of the light. 
```C++
light.pullConfig();
bool lightState = light.getState();
int lightDim = light.getDimming();
int lightTemperature = light.getTemperature();
// you can alo get struct that contains the full configuration.
LightConfig lightConfig = light.getConfig();
```


## Dependencies
- [ArduinoJson](https://arduinojson.org/)

## Notes
Please note, that I was only able to test this code with the RGB version of the Wiz lights. 
Parameters, such as the minimum and maximum dimming values or light temperatures might be different for other models. 
Please let me know if you have difficulties with other light bulb models. 
