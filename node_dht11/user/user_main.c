/* main.c -- MQTT client example
*
* Copyright (c) 2014-2015, Tuan PM <tuanpm at live dot com>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* * Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
* * Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* * Neither the name of Redis nor the names of its contributors may be used
* to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/
#include "ets_sys.h"
#include "json/json.h"
#include "json/jsonparse.h"
#include "driver/uart.h"
#include "osapi.h"
#include "mqtt.h"
#include "wifi.h"
#include "config.h"
#include "debug.h"
#include "gpio.h"
#include "string.h"
#include "user_interface.h"

static volatile os_timer_t send_timer;
#define MAXTIMINGS 10000
#define BREAKTIME 20
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"

static volatile float lastHum, lastTemp;
static volatile os_timer_t some_timer;
static char hwaddr[6];

MQTT_Client mqttClient;

void wifiConnectCb(uint8_t status)
{
	if(status == STATION_GOT_IP){
        char *channel = DOMOTICKS_CHANNEL;
		MQTT_Connect(&mqttClient);
		MQTT_Subscribe(&mqttClient, channel);
	}
}
void mqttConnectedCb(uint32_t *args)
{
	MQTT_Client* client = (MQTT_Client*)args;
	INFO("MQTT: Connected\r\n");

}

void mqttDisconnectedCb(uint32_t *args)
{
	MQTT_Client* client = (MQTT_Client*)args;
	INFO("MQTT: Disconnected\r\n");
}

void mqttPublishedCb(uint32_t *args)
{
	MQTT_Client* client = (MQTT_Client*)args;
	client->
	INFO("MQTT: Published\r\n");
}

void mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len)
{
	char topicBuf[64], dataBuf[64];
	struct jsonparse_state json_struct;
	memset(&json_struct, NULL, sizeof(struct jsonparse_state));

	MQTT_Client* client = (MQTT_Client*)args;

	os_memcpy(topicBuf, topic, topic_len);
	topicBuf[topic_len] = 0;

	os_memcpy(dataBuf, data, data_len);
	dataBuf[data_len] = 0;

	if(os_strstr(topicBuf, DOMOTICKS_UUID) != NULL){
		// This message is for me!
		INFO("MQTT topic: %s, data: %s \r\n", topicBuf, dataBuf);
		jsonparse_setup(&json_struct, dataBuf, data_len);
		while(jsonparse_next(&json_struct)){

		}
	}


	/* Echo back to /echo channel*/
	MQTT_Publish(client, "/echo", dataBuf, data_len, 0, 0);
}

void sendData(void *arg){
	char msg[64];
	os_sprintf(msg, "Humidity: %d & Temperature : %d\0", lastHum, lastTemp);

	MQTT_Publish(&mqttClient, DOMOTICKS_CHANNEL, msg, os_strlen(msg), 0, 0);
}

void ICACHE_FLASH_ATTR readDHT(void *arg)
{

    int counter = 0;
    int laststate = 1;
    int i = 0;
    int j = 0;
    int checksum = 0;
    //int bitidx = 0;
    //int bits[250];

    int data[100];
    os_printf("\r\nDTH11 reading ...\r\n");
    data[0] = data[1] = data[2] = data[3] = data[4] = 0;

    GPIO_OUTPUT_SET(2, 1);
    os_delay_us(250000);
    GPIO_OUTPUT_SET(2, 0);
    os_delay_us(20000);
    //os_intr_lock();
    GPIO_OUTPUT_SET(2, 1);
    os_delay_us(40);
    GPIO_DIS_OUTPUT(2);
    PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO2_U);


    // wait for pin to drop?
    while (GPIO_INPUT_GET(2) == 1 && i<100000) {
          os_delay_us(1);
          i++;
    }

	if(i==100000){
		return;
	}

    // read data!

    for (i = 0; i < MAXTIMINGS; i++) {
        counter = 0;
        while ( GPIO_INPUT_GET(2) == laststate) {
            counter++;
			os_delay_us(1);
            if (counter == 255)
                break;
        }
        laststate = GPIO_INPUT_GET(2);
        if (counter == 255) break;

        //bits[bitidx++] = counter;

        if ((i > 3) && (i%2 == 0)) {
            // shove each bit into the storage bytes
            data[j/8] <<= 1;
            if (counter > BREAKTIME)
                data[j/8] |= 1;
            j++;
        }
    }

    //os_intr_unlock();

    /*for (i=3; i<bitidx; i+=2) {
        os_printf("bit %d: %d\n", i-3, bits[i]);
        os_printf("bit %d: %d (%d)\n", i-2, bits[i+1], bits[i+1] > BREAKTIME);
    }*/

    float temp_p, hum_p;
    if (j >= 39) {
        checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
        if (data[4] == checksum) {
            /* yay! checksum is valid */
        	lastHum = (float)data[0];
        	lastTemp = (float)data[2];
        	/* DHT22 Stuff... */
            /*hum_p = data[0] * 256 + data[1];
            hum_p /= 10;

            temp_p = (data[2] & 0x7F)* 256 + data[3];
            temp_p /= 10.0;
            if (data[2] & 0x80)
                temp_p *= -1;*/

            // End DHT22 Stuff
            //sendReading(temp_p, hum_p);
           /* lastHum = hum_p;
            lastTemp = temp_p;*/

        	char msg[64];
        	MQTT_Publish(&mqttClient, DOMOTICKS_CHANNEL, msg, os_strlen(msg), 0, 0);
            /*os_printf("Hum : %d, Temp : %d", data[0], data[2]);
            os_printf("Hum : %d, Temp : %d", lastHum, lastTemp);*/
        }
    }
}



void user_init(void)
{
	lastHum = 0;
	lastTemp=0;
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	os_delay_us(1000000);

    //Set GPIO2 to output mode
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
    PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO2_U);

	CFG_Load();

	MQTT_InitConnection(&mqttClient, sysCfg.mqtt_host, sysCfg.mqtt_port, SEC_NONSSL);
	MQTT_InitClient(&mqttClient, sysCfg.device_id, sysCfg.mqtt_user, sysCfg.mqtt_pass, sysCfg.mqtt_keepalive);
	MQTT_OnConnected(&mqttClient, mqttConnectedCb);
	MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
	MQTT_OnPublished(&mqttClient, mqttPublishedCb);
	//MQTT_OnData(&mqttClient, mqttDataCb);

	WIFI_Connect(sysCfg.sta_ssid, sysCfg.sta_pwd, wifiConnectCb);

	INFO("\r\nSystem started ...\r\n");

    //Set GPIO2 to output mode
    /*PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
    PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO2_U);
     */

    os_timer_disarm(&some_timer);
    //Setup timer
    os_timer_setfn(&some_timer, (os_timer_func_t *)readDHT, NULL);
    //os_timer_disarm(&send_timer);
    //os_timer_setfn(&send_timer, (os_timer_func_t *)sendData, NULL);
    //Arm the timer
    //&some_timer is the pointer
    //1000 is the fire time in ms
    //0 for once and 1 for repeating

    os_timer_arm(&some_timer, 10000, 1);
    //os_timer_arm(&send_timer, 15000, 1);
}
