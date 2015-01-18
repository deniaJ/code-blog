#include "dht11.h"

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
		os_printf("Never drop...");
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

    os_printf("Data (%d): 0x%x 0x%x 0x%x 0x%x 0x%x\n", j, data[0], data[1], data[2], data[3], data[4]);

    float temp_p, hum_p;
    if (j >= 39) {
        checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
        if (data[4] == checksum) {
            /* yay! checksum is valid */
        	/* DHT22 Stuff... */
            /*hum_p = data[0] * 256 + data[1];
            hum_p /= 10;

            temp_p = (data[2] & 0x7F)* 256 + data[3];
            temp_p /= 10.0;
            if (data[2] & 0x80)
                temp_p *= -1;*/

            // End DHT22 Stuff
            //sendReading(temp_p, hum_p);
            os_printf("Hum : %d, Temp : %d", data[0], data[2]);
        }
    }
    else{
    	os_printf("j <= 40 :/ %d", j);
    }

}
