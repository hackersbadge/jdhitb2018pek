/*
 * Copyright (c) 2015, Vladimir Komendantskiy
 * MIT License
 *
 * SSD1306 demo of block and font drawing.
 */

//
// fixed for OrangePiZero by HypHop
//

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <pthread.h>

#include "oled.h"
#include "font.h"

#define I2C_DEV     "/dev/i2c-0"
#define ACTIVE_SH   "/etc/shepherd-active.sh"

static void *blink_the_led(void * arg)
{
    int ret;
    int led[2] = {41, 40};
    char cmd[64];
    int i, j, k;
/* flag:6a646869746232303138 */
    uint8_t flag[10] = {0x6a, 0x64, 0x68, 0x69, 0x74, 0x62, 0x32, 0x30, 0x31, 0x38};
    uint8_t x;

    for (i = 0; i < 2; i++) {
        snprintf(cmd, sizeof(cmd), "echo %d > /sys/class/gpio/export", led[i]);
        system(cmd);
        usleep(10000);
        snprintf(cmd, sizeof(cmd), "echo out > /sys/class/gpio/gpio%d/direction", led[i]);
        system(cmd);
        usleep(10000);
        snprintf(cmd, sizeof(cmd), "echo 1 > /sys/class/gpio/gpio%d/value", led[i]);
        system(cmd);
        usleep(10000);
        snprintf(cmd, sizeof(cmd), "/sys/class/gpio/gpio%d/value", led[i]);
    }

    sleep(5);
    for(k = 0; k < 10; k++) {
        for(i = 0; i < 10; i++) {
            for(j = 7; j >= 0; j--) {
                x = flag[i];
                x = x << (7 - j);
                x = x >> 7;
                if (x == 0) {
                    snprintf(cmd, sizeof(cmd), "echo 1 > /sys/class/gpio/gpio%d/value", led[1]);
                    system(cmd);
                    snprintf(cmd, sizeof(cmd), "echo 0 > /sys/class/gpio/gpio%d/value", led[0]);
                    system(cmd);
                } else {
                    snprintf(cmd, sizeof(cmd), "echo 1 > /sys/class/gpio/gpio%d/value", led[0]);
                    system(cmd);
                    snprintf(cmd, sizeof(cmd), "echo 0 > /sys/class/gpio/gpio%d/value", led[1]);
                    system(cmd);
                }
                usleep(800000);
                if (x == 0) {
                    snprintf(cmd, sizeof(cmd), "echo 1 > /sys/class/gpio/gpio%d/value", led[0]);
                    system(cmd);
                } else {
                    snprintf(cmd, sizeof(cmd), "echo 1 > /sys/class/gpio/gpio%d/value", led[1]);
                    system(cmd);
                }
                usleep(200000);
            }
            snprintf(cmd, sizeof(cmd), "echo 1 > /sys/class/gpio/gpio%d/value", led[0]);
            system(cmd);
            snprintf(cmd, sizeof(cmd), "echo 1 > /sys/class/gpio/gpio%d/value", led[1]);
            system(cmd);
            sleep(3);
        }
        snprintf(cmd, sizeof(cmd), "echo 1 > /sys/class/gpio/gpio%d/value", led[0]);
        system(cmd);
        snprintf(cmd, sizeof(cmd), "echo 1 > /sys/class/gpio/gpio%d/value", led[1]);
        system(cmd);
        sleep(10);
    }

    return NULL;
}

static int uci_get(const char *key, char *value, int value_len)
{
    int ret;
    FILE *fp;
    char cmd[64];

    snprintf(cmd, sizeof(cmd), "uci get %s", key);
    fp = popen(cmd, "r");
    if (fp == NULL) {
        return -1;
    }
    fgets(value, value_len, fp);
    ret = strlen(value);
    if (ret == 0) {
        return -2;
    } else {
        /* remove tail LF */
        value[ret - 1] = 0;
    }
    pclose(fp);

    return 0;
}

int main(int argc, char **argv) {
	struct display_info disp;
    int ret;
    char ssid[16];
    char key[16];
    struct stat st;
    FILE *fp;
    char buf[64];
    pthread_t pt;
    void *thread_ret;

	memset(&disp, 0, sizeof(disp));
	disp.address = OLED_I2C_ADDR;
	disp.font = font2;

    for (;;) {
        ret = oled_open(&disp, I2C_DEV);
        if (ret < 0) {
            fprintf(stderr, "oled open i2c device error.\n");
            continue;
            sleep(1);
        }
        ret = oled_init(&disp);
        if (ret < 0) {
            fprintf(stderr, "oled init error.\n");
            oled_close(&disp);
            sleep(1);
            continue;
        } else {
            break;
        }
    }

    /* check the flag */
    ret = stat(ACTIVE_SH, &st);
    if (ret == 0) {
        /* show the default string */
        disp.font = font2;
        oled_putstrto(&disp, 4, 4, "See Me at Lunch Area");
        oled_putstrto(&disp, 4, 20, "  12:00 till 14:00");
        oled_send_buffer(&disp);
        /* up the wifi dev */
        system("ifconfig wlan0 up");
        /* wait active signal */
        for(;;) {
            printf("scan the wifi...\n");
            /* scan the wifi shepherd-lab */
            fp = popen("iw wlan0 scan | grep 'SSID: theshepherdlab.io' | wc -l", "r");
            if (fp == NULL) {
                sleep(1);
                continue;
            } else {
                fgets(buf, sizeof(buf), fp);
                pclose(fp);
                ret = atoi(buf);
                if (ret != 0) {
                    break;
                }
            }
            sleep(1);
        }
        system(ACTIVE_SH);
    }
    /* actived. set the flag and blink led light */
    pthread_create(&pt, NULL, blink_the_led, NULL);
    /* else show ssid and key */
    memset(ssid, 0, sizeof(ssid));
    memset(key, 0, sizeof(key));
    for(;;) {
        ret = uci_get("wireless.default_radio0.ssid", ssid, sizeof(ssid));
        if (ret) {
            sleep(1);
            continue;
        }
        ret = uci_get("wireless.default_radio0.key", key, sizeof(key));
        if (ret) {
            sleep(1);
            continue;
        }
        break;
    }
    printf("get wireless ssid[%s], key[%s].\n", ssid, key);

    disp.font = font2;
    oled_putstrto(&disp, 4, 4, "SSID: ");
    oled_putstrto(&disp, 4 + 36, 4, ssid);
    oled_putstrto(&disp, 4, 20, "KEY:  ");
    oled_putstrto(&disp, 4 + 36, 20, key);
    oled_send_buffer(&disp);

    pthread_join(pt, &thread_ret);

	return 0;
}
