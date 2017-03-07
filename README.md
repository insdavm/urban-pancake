## Living Room Lamp Automation
### w/ the NodeMCU v1.0 (Amica R2) ESP8266 Breakout

----

#### Overview
This program uses an [NodeMCU Devkit v1.0](https://github.com/nodemcu/nodemcu-devkit-v1.0) ESP8266 breakout to connect to WiFi, connect to an [MQTT](https://en.wikipedia.org/wiki/MQTT) broker, and then listen for MQTT traffic.  When the string ```ON``` is published to the MQTT topic ```openhab/livingroom/relay1```, pin 16 is pulled high, turning a 3v ICStation relay on and, in turn, allowing 120VAC to run to the lamp (which has to be switched "on" at all times for the relay to work properly).  When ```OFF``` is received through the same topic, the pin is pulled LOW and the relay turns off.

#### Temperature Reporting

The program will report the temperature from the [TMP36 temperature sensor](https://www.sparkfun.com/products/10988) every **10 minutes**.  The Analog-to-Digital Converter (ADC) on the ESP8266 module is a 10-bit ADC with a max voltage input of 1v.  This particular breakout includes, however, a 220/100k voltage divider built into the board just before the ADC pin.  This means the max voltage is actually around 3.3v, or VCC.  In testing however, I've found that the actual aRef is about 3406 mV, which equates to 1604 mV (ESP12-E ADC aRef) after the RVD does it's job.

To calculate the temperature from the voltage, we use a formula corresponding with the data in the datasheet, which states that the range is 100mV - 2000mV, with 100mV equating to -40 deg C and 2000mV equating to 125 deg C.  1 deg C equals exactly 10mV so, 0 deg C would be 500mV, for example.

```
TEMP = analogRead(ADC_PIN) * 3406.0;      /* multiply ADC reading (1-1024) by the Analog Reference Voltage (3406mV in this case) */
TEMP /= 1024;                             /* get millivolts, but... */
TEMP *= 2;                                /* ...we divided the voltage down by 2 with our RVD, so multiply back up to simulate a 100mv - 2000mv range */
TEMP -= 500;                              /* subtract 500mV to get us to 0 deg C */
TEMP /= 10;                               /* YIELD: Degrees C  (divide by 10 because it's 1 deg C per 10 mV) */
TEMP *= 1.8;
TEMP += 32;                               /* YIELD: Degrees F  (multiply by 1.8 and add 32) */
```
----

##### Author
[Austin](insdavm@gmail.com)
