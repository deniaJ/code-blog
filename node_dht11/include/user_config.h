#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_
#include "user_interface.h"

#define CFG_HOLDER	0x00FF55A5
#define CFG_LOCATION	0x3C	/* Please don't change or if you know what you doing */

/*DEFAULT CONFIGURATIONS*/

#define MQTT_HOST			"192.168.1.106" //or "192.168.11.1"
#define MQTT_PORT			1883
#define MQTT_BUF_SIZE		1024
#define MQTT_KEEPALIVE		120	 /*second*/

#define MQTT_CLIENT_ID		"DVES_%08X"
#define MQTT_USER			"DVES_USER"
#define MQTT_PASS			"DVES_PASS"

#define STA_SSID "SSID"
#define STA_PASS "Passwd"
#define STA_TYPE AUTH_WPA2_PSK

#define DOMOTICKS_UUID "f8c28d22-9406-11e4-89bf-fff6ce094a8a"

#define DOMOTICKS_CHANNEL "/domoticks/esp8266/f8c28d22-9406-11e4-89bf-fff6ce094a8a"

#define MQTT_RECONNECT_TIMEOUT 	5	/*second*/
#define MQTT_CONNTECT_TIMER 	5 	/**/

#define CLIENT_SSL_ENABLE
#endif
