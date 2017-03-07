/**
 * urban-pancake
 *
 * ...an ESP8266 (NodeMCU Amica / Alloet ESP-12E Breakout) powered
 * living room lamp controller
 *
 * @author Austin <austin@github.com>
 * @date 2017.02.28
 */

#ifndef URBAN_PANCAKE_H
#define URBAN_PANCAKE_H

#define    RELAY         16       /* "D0" on the board */
#define    ADC           A0       /* ADC pin, "A0" on the board xs*/

void callback(char*, byte*, unsigned int);
void setupWiFi(void);
void reconnect(void);
void publishTemperature(void);
void publishState(void);

#endif
