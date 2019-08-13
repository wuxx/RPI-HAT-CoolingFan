#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include <wiringPi.h>

/* gpio config */
#define HC595_RCLK  (28)
#define HC595_CLK   (27)
#define HC595_SER   (29)

/* seven segment digital display */
/* common negative */

/*
#     __A_
#    |     |      |  
#  F |     | B    |  
#    |__G__|      |  
#    |     |      |  
#  E |     | C    |  
#    |__D__|  .DP |  
*/

enum DIGIT_INDEX_E {
    D1 = 0x0E,
    D2 = 0x0D,
    D3 = 0x0B,
    D4 = 0x07,
};

struct ssd_map {
    uint8_t A;
    uint8_t B;
    uint8_t C;
    uint8_t D;
    uint8_t E;
    uint8_t F;
    uint8_t G;
    uint8_t DP;
};

struct ssd_map ssd_table[16] = {
    /* 0 */ {.A = 1, .B = 1, .C = 1, .D = 1, .E = 1, .F = 1, .G = 0, .DP = 0},
    /* 1 */ {.A = 0, .B = 1, .C = 1, .D = 0, .E = 0, .F = 0, .G = 0, .DP = 0},
    /* 2 */ {.A = 1, .B = 1, .C = 0, .D = 1, .E = 1, .F = 0, .G = 1, .DP = 0},
    /* 3 */ {.A = 1, .B = 1, .C = 1, .D = 1, .E = 0, .F = 0, .G = 1, .DP = 0},
    /* 4 */ {.A = 0, .B = 1, .C = 1, .D = 0, .E = 0, .F = 1, .G = 1, .DP = 0},
    /* 5 */ {.A = 1, .B = 0, .C = 1, .D = 1, .E = 0, .F = 1, .G = 1, .DP = 0},
    /* 6 */ {.A = 1, .B = 0, .C = 1, .D = 1, .E = 1, .F = 1, .G = 1, .DP = 0},
    /* 7 */ {.A = 1, .B = 1, .C = 1, .D = 0, .E = 0, .F = 0, .G = 0, .DP = 0},
    /* 8 */ {.A = 1, .B = 1, .C = 1, .D = 1, .E = 1, .F = 1, .G = 1, .DP = 0},
    /* 9 */ {.A = 1, .B = 1, .C = 1, .D = 1, .E = 0, .F = 1, .G = 1, .DP = 0},

    /* A */ {.A = 1, .B = 1, .C = 1, .D = 0, .E = 1, .F = 1, .G = 1, .DP = 0},
    /* B */ {.A = 1, .B = 1, .C = 1, .D = 1, .E = 1, .F = 1, .G = 1, .DP = 0},
    /* C */ {.A = 1, .B = 0, .C = 0, .D = 1, .E = 1, .F = 1, .G = 0, .DP = 0},
    /* D */ {.A = 1, .B = 1, .C = 1, .D = 1, .E = 1, .F = 1, .G = 0, .DP = 0},
    /* E */ {.A = 1, .B = 0, .C = 0, .D = 1, .E = 1, .F = 1, .G = 1, .DP = 0},
    /* F */ {.A = 1, .B = 0, .C = 0, .D = 0, .E = 1, .F = 1, .G = 1, .DP = 0},
};

/* 4 digital data */
uint16_t ssd_data_all;

char sys_banner[] = {"rpi_hat coolingfan system buildtime [" __TIME__ " " __DATE__ "] " "rev 1.0"}; 

void msleep(unsigned int ms)
{
    int i;
    for(i = 0; i < ms; i++) {
        usleep(i * 1000);
    }
}

#define get_bit(x, bit_index) ((x >> bit_index) & 0x1)

void set_bit(uint8_t *x, uint8_t bit_index, uint8_t b)
{
    uint8_t _x;
    uint8_t bit_mask;
    _x = *x;
    if (get_bit(_x, bit_index) != b) {
        if (b == 0) {
            bit_mask = ~(0x1 << bit_index);
            *x = (_x) & bit_mask;
        } else {    /* b == 1 */
            bit_mask = (0x1 << bit_index);
            *x = (_x) | bit_mask;
        }
    }
}


/* 74HC595 */
void hc595_set_rclk(uint8_t b)
{
    if (b) {
        digitalWrite(HC595_RCLK, HIGH);
    } else {
        digitalWrite(HC595_RCLK, LOW);
    }
}

void hc595_set_clk(uint8_t b)
{
    if (b) {
        digitalWrite(HC595_CLK, HIGH);
    } else {
        digitalWrite(HC595_CLK, LOW);
    }
}

void hc595_set_ser(uint8_t b)
{
    if (b) {
        digitalWrite(HC595_SER, HIGH);
    } else {
        digitalWrite(HC595_SER, LOW);
    }
}

int hc595_set_data(uint16_t data)
{
    uint8_t b;
    int8_t i;

    hc595_set_rclk(0);

    for(i = 15; i >= 0; i--) {
        if (data & (0x1 << i)) {
            b = 1;
        } else {
            b = 0;
        }

        //printf("%d\n", b);

        hc595_set_clk(0);
        hc595_set_ser(b);
        hc595_set_clk(1);
    }

    hc595_set_rclk(1);

    return 0;
}

int8_t hc595_init()
{

    wiringPiSetup();

    pinMode(HC595_RCLK, OUTPUT);
    pinMode(HC595_CLK,  OUTPUT);
    pinMode(HC595_SER,  OUTPUT);


    hc595_set_rclk(0);
    hc595_set_clk(0);
    hc595_set_ser(0);

    return 0;
}


uint8_t num2data(uint8_t num, uint8_t dp_on)
{
    uint8_t display_data;
    num = num % 16;

    set_bit(&display_data, 7, ssd_table[num].A);
    set_bit(&display_data, 6, ssd_table[num].B);
    set_bit(&display_data, 5, ssd_table[num].C);
    set_bit(&display_data, 4, ssd_table[num].D);
    set_bit(&display_data, 3, ssd_table[num].E);
    set_bit(&display_data, 2, ssd_table[num].F);
    set_bit(&display_data, 1, ssd_table[num].G);

    if (dp_on) {
        set_bit(&display_data, 0, 1);
    } else {
        set_bit(&display_data, 0, ssd_table[num].DP);
    }

    return display_data;

}

int ssd_set(uint8_t dindex, uint8_t num, uint8_t dp_on)
{
    uint16_t data;
    data = (dindex << 8) | num2data(num, dp_on);
    hc595_set_data(data);
    return 0;
}

uint8_t SSD_DATA[5];

uint32_t get_temp()
{
    char buf[16] = {0};
    int tfd = -1;
    uint32_t temp = 0;

    if ((tfd = open("/sys/class/thermal/thermal_zone0/temp", O_RDONLY)) == -1) {
        perror("open");
        exit(-1);
    }

    memset(buf, 0, sizeof(buf));

    read(tfd, buf, sizeof(buf));
    
    //printf("read [%s]\r\n", buf);
    sscanf(buf, "%d", &temp);
    //printf("temp: %d\r\n", temp);

    close(tfd);

    return (temp / 100);
}

int32_t get_ip(uint8_t *ip)
{
    uint8_t *p;
    struct ifreq ifr;
    static int sock_fd = -1;    /* FIXME: close sock_fd */

    if (sock_fd == -1) {
        sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    }


    strcpy(ifr.ifr_name, "wlan0");
    if (ioctl(sock_fd, SIOCGIFADDR, &ifr) <  0) {
        printf("ioctl fail\n");
        return -1;
    }

    p = (uint8_t *)&(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr);

    printf("ip: %d.%d.%d.%d\n", p[0], p[1], p[2], p[3]);

    memcpy(ip, p, 4);

    return 0;
}

int ssd_display_temp()
{
    ssd_set(D1, SSD_DATA[1], 1); /* the 1 is guessed. */
    ssd_set(D2, SSD_DATA[2], 0);
    ssd_set(D3, SSD_DATA[3], 0);
    ssd_set(D4, SSD_DATA[4], 0);

    return 0 ;
}

int ssd_display_ip()
{
    /* D1 keep off */

    if (SSD_DATA[2] != 0) {
        ssd_set(D2, SSD_DATA[2], 0);
    }

    if ((SSD_DATA[2] != 0) || SSD_DATA[3] != 0) {
        ssd_set(D3, SSD_DATA[3], 0);
    }

    ssd_set(D4, SSD_DATA[4], 0);

    return 0 ;
}

int ssd_display_off()
{
    hc595_set_data(0xFFFF);
    return 0;
}


/* 0: show temp; 1: show ip; */
enum THREAD_TASK_TYPE_E {
    SHOW_TEMP = 0,
    SHOW_IP,
    SHOW_OFF,
};

uint32_t task_type = SHOW_OFF;

void * thread_ssd_display(void *arg)
{
    printf("enter %s-%d\n", __func__, __LINE__);

    while(1) {
        switch (task_type) {
            case (SHOW_TEMP):
                ssd_display_temp();
                break;
            case (SHOW_IP):
                ssd_display_ip();
                break;
            case (SHOW_OFF):
                ssd_display_off();
                break;
        }
    }

    return NULL;
}


void task_display_temp()
{

    uint32_t temp;
    uint32_t count = 20;

    task_type = SHOW_TEMP;

    while (count --) {

        temp = get_temp();
        printf("get_temp: %d\r\n", temp);

        SSD_DATA[1] = temp / 100;
        SSD_DATA[2] = (temp / 10) % 10;
        SSD_DATA[3] = temp % 10;
        SSD_DATA[4] = 0xC;

        usleep(500 * 1000);

    }


}

void task_display_ip()
{
    uint8_t ip[4];

    task_type = SHOW_IP;

    get_ip(ip);

    SSD_DATA[2] = ip[0] / 100;
    SSD_DATA[3] = (ip[0] / 10) % 10;
    SSD_DATA[4] = ip[0] % 10;

    sleep(1);

    SSD_DATA[2] = ip[1] / 100;
    SSD_DATA[3] = (ip[1] / 10) % 10;
    SSD_DATA[4] = ip[1] % 10;

    sleep(1);

    SSD_DATA[2] = ip[2] / 100;
    SSD_DATA[3] = (ip[2] / 10) % 10;
    SSD_DATA[4] = ip[2] % 10;

    sleep(1);

    SSD_DATA[2] = ip[3] / 100;
    SSD_DATA[3] = (ip[3] / 10) % 10;
    SSD_DATA[4] = ip[3] % 10;

    sleep(1);

}

int main() 
{
	int r;
    pthread_t th;
    uint8_t ip[4];

    hc595_init();

    printf("%s\r\n", sys_banner);

    if((r = pthread_create(&th, NULL, thread_ssd_display, NULL)) != 0) {
        printf( "pthread_create fail!\n");
        return -1;
    }


    //ssd_display_temp(123);
    //get_temp();

    get_ip(ip);

    while (1) {
        task_display_temp();

        task_display_ip();
    }

    while (1) {
        ssd_set(D1, 1, 0);
        ssd_set(D2, 2, 0);
        ssd_set(D3, 3, 0);
        ssd_set(D4, 4, 0);
    }

    while(1);

    return 0;
}
