/**
 * urban-pancake
 *
 * ...an ESP8266 (NodeMCU Amica / Alloet ESP-12E Breakout) powered
 * living room lamp controller
 *
 * @author Austin <austin@github.com>
 * @date 2017.02.28
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>

#include "urban-pancake.h"

const char    *ESSID                =   "your-essid";                      /* ESSID */
const char    *PASSWORD             =   "your-password";                   /* WPA2-PSK Passphrase */
const char    *HOSTNAME             =   "esp8266-livingroom";              /* DHCP hostname */
const char    *MQTT_SERVER          =   "your-mqtt-broker";                /* IP address or FQDN of MQTT broker */
const short    MQTT_PORT            =   1883;                              /* Port that MQTT broker is listening on */
const char    *MQTT_SUBSCRIBE_TOPIC =   "openhab/livingroom/relay1";       /* MQTT topic to subscribe to for commands */
const char    *MQTT_PUB_TEMP_TOPIC  =   "openhab/livingroom/temperature";  /* MQTT topic to publish the temperature to */
const char    *MQTT_PUB_STATE_TOPIC =   "openhab/livingroom/state";        /* MQTT topic to publish the relay state to */
unsigned long tempTime              =   0;
unsigned long tempInterval          =   600000;                            /* 10 min */
unsigned long stateTime             =   0;
unsigned long stateInterval         =   60000;                             /* 1 min */

WiFiClient espClient;
PubSubClient client(espClient);

/**
 * Callback function for when a message is received by the 
 * MQTT client on the subscribed topic
 *
 * @param topic the topic
 * @param payload the actual message
 * @param length the length of the message in bytes
 */
void callback(char* topic, byte* payload, unsigned int length)
{
  char buffer[length+1];
  int i = 0;
  
  while (i<length) {
    buffer[i] = payload[i];
    i++;
  }
  buffer[i] = '\0';

  String msgString = String(buffer);
  Serial.println("Inbound: " + String(topic) + ":" + msgString);

  if ( msgString == "ON" ) {
    digitalWrite(RELAY, HIGH);         /* relay on when 3v3 is applied */
  } else if ( msgString == "OFF" ) {
    digitalWrite(RELAY, LOW);          /* relay off when 0v is applied */
  }
  publishState();                      /* so OpenHAB will make the lightbulb icon reflect the state of the lamp */              
}

/**
 * Get the ESP8266's 2.4gz 802.11 b/g/n radio connected
 * to the network so we can actually receive MQTT traffic
 */
void setupWiFi()
{ 
  Serial.println();

  WiFi.hostname(HOSTNAME);

  /*
   * For some reason this defaults to AP_STA, so we need 
   * to explicitly turn the AP functionality off 
   */
  WiFi.mode(WIFI_STA); 
  WiFi.begin(ESSID, PASSWORD);

  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Hostname: ");
  Serial.print(WiFi.hostname());
  Serial.println();
}

/**
 * Attempt reconnection to the MQTT broker and subscription
 * to the MQTT topic indefinitely
 */
void reconnect()
{
  while (!client.connected()) {
    
    Serial.print("Attempting MQTT connection...");
    String clientId = WiFi.hostname();
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(MQTT_SUBSCRIBE_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
    
  }
}

/**
 * Get temperature from TMP36 sensor and publish
 * to the MQTT topic defined in mqttPubTopic
 */
void publishTemperature()
{
  int adcReading = analogRead(ADC);        /* get reading from ADC, 0 <= adcReading <= 1024 */
  
  float mV = adcReading * 3406.0;          /* Actual aRef was calculated as being around 3406 mV... */
  mV /= 1024.0;

  /* 
   * we divided the voltage with a 10k/10k RVD, so multiply it back up...
   *
   * in hindsight this was unnecessary, because the NodeMCU board has a built in 
   * 220k/100k RVD right before the ADC pin, so we would have been fine connecting
   * the TMP36 directly.  Oh well, we lost 2mV resolution, so what...
   */
  mV *= 2.0;                               

  /* Celsius = ((mV - 500) / 10), Fahrenheit = (C * 1.8) + 32 */
  float tempF = ( ((mV - 500) / 10) * 1.8 ) + 32;
  
  Serial.print("Publishing temperature: ");
  Serial.println(tempF);
  
  /* convert tempF to a char array and publish to MQTT */
  char charArray[10];
  String tempFString = String(tempF);
  tempFString.toCharArray(charArray, tempFString.length()+1);
  client.publish(MQTT_PUB_TEMP_TOPIC, charArray);
}

/** 
 * Get state of relay and publish it to MQTT
 */
void publishState()
{
  char *state = (char *) "";                    
  if ( digitalRead(RELAY) == HIGH ) {           /* pin is at 3v3, relay is on */
    state = (char *) "ON";
    Serial.println("Relay is on.");
  } else if ( digitalRead(RELAY) != HIGH ) {   /* pin is at 0v, relay is off */
    state = (char *) "OFF";
    Serial.println("Relay is off.");
  }
  client.publish(MQTT_PUB_STATE_TOPIC, state);
}

void setup()
{
  pinMode(RELAY, OUTPUT);                       /* set relay pin to output */
  digitalWrite(RELAY, LOW);                     /* relay is off when 0v is applied */
  Serial.begin(115200);                         /* start serial connection for debugging */
  setupWiFi();                                  /* start WiFi connection */
  client.setServer(MQTT_SERVER, MQTT_PORT);     /* set the MQTT broker address and port */
  client.setCallback(callback);                 /* set the function to use when a message is received */
}


void loop()
{ 
  /* Send temperature if it's time to do so */
  if ( millis() - tempTime > tempInterval ) {
    tempTime = millis();
    publishTemperature();
  }

  /* Send state of relay if it's time to do so */
  if ( millis() - stateTime > stateInterval ) {
    stateTime = millis();
    publishState();
  }
  
  /* Make sure we're connected to the MQTT broker */
  if (!client.connected()) {
    reconnect();
  }

  /* Monitor for MQTT traffic */
  client.loop();
}
