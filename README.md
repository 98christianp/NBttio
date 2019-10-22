# NBttio

Guide for setting up a Sodaq NB-IoT device up on Thethings.io

For RGB LED, connect red pin to D2, green -> D3 blue -> D4

This repository will contains 2 files as well as this guide. The file main.cpp  should be uploaded to the Sodaq board via Arduino and the other parser.js, which should be copied into thethings.io cloud code. 

# Guide
First of all, set up a thethings.io account. For help on this, look up the official guide, https://developers.thethings.io/docs/getting-started.

## Set up cloud code 
The cloud code designates a key value system to the UDP payload. In the included example, the geoloc variable reads tbe $geo location of the device with the token D9s.... 

The key value is important as this is how you can access the data through APIs and on the dashboard.

For the example, the key "signal" in the function the value is from the AT+CSQ command on the sodaq. I set it up on the way that the first 2 bytes of the payload represent this variable, and that is what we parse with `parseInt(params.data.substring(0,2), 16)` 

## Set up Arduino code/Sodaq device
For the arduino, make sure to have the libraries that are included installed. 

Sodaq_UBlox_GPS.h and
Sodaq_wdt.h

Then update APN/ForceOperator if needed (example uses Telia) update thethings.io token with your own device as well. 

The `build_message` function is where we gain connectivity details and format it to be sent as HEX. 

## Brief explanation of how to send other sensor data
If you have sensor data it is crucial to conver the value you get as hexadecimal, furthermore you should know how many bytes it will be. I.E the decimal value 15 would be one byte (0x0F) in hex. This is done with `String(decimal, HEX)`, be cautios because  arudino doesn't add the 0 infront automatically, but the U-Blox AT command requires the length to be evenly lengthed. So sometimes you have to manually prepend a "0" to this. 

`build_message()` returns a string:

```cpp
return(result+gps_string+prepend+s2+prepend+s1);
```

The result comes from "AT+CSQ" and is 2 bytes. The gps_string is 12 bytes and s2 is the cell id and s1, it's 10 bytes, s2 is the signal power and is 4 bytes. These are both returned from "AT+NUESTATS"

You can either add to this return your own variables or change it entirely. Just note that you should add a Key:Value to to cloud code where in the value you would parse only that added substring, for example if I wanted to add a temperature reading that is 2 bytes:

```cpp
// I add the temperature string
return(result+gps_string+prepend+s2+prepend+s1+temperature);
```

```javascript
        // I add the temperature key and value:
        //the key must not contain whitespaces!
        {
             "key":"Temperature",
             // 26-28 i.e the next 2 bytes after signal power
             "value": - parseInt(params.data.substring(26,28),16)/10,
            "geo":
            {
                "lat":parseInt(params.data.substring(2,8),16)/10000,
                "long":parseInt(params.data.substring(8,14),16)/10000
             }
        }
```