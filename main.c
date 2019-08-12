#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

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

struct ssd_map ssd_table[10] = {
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

/* AT28C256 */
void at28_set_addr(uint16_t addr)
{
    uint8_t b;
    int8_t i;

    hc595_set_rclk(0);

    for(i = 15; i >= 0; i--) {
        if (addr & (0x1 << i)) {
            b = 1;
        } else {
            b = 0;
        }

        //printf("%d\n", b);

        hc595_set_clk(0);
        hc595_set_ser(b);
        msleep(3);
        hc595_set_clk(1);
        msleep(3);
    }

    msleep(2);
    hc595_set_rclk(1);

}

uint8_t at28_read(uint16_t addr)
{
#if 0
    int8_t i;
    uint8_t data = 0;

    at28_set_ce(0);
    at28_set_oe(0);
    msleep(1);

    at28_set_addr(addr);
    msleep(1);

    for(i = 7; i >= 0; i--) {
        data = data << 1;
        data |= gpio_read(pgi_at28_io[i]->base_addr, pgi_at28_io[i]->index);
    }

    at28_set_ce(1);
    at28_set_oe(1);

    return data;
#endif
    return 0;
}

void at28_write(uint16_t addr, uint8_t data)
{
#if 0
    uint8_t i;

    at28_set_addr(addr);

    for(i = 0; i < 8; i++) {

        gpio_init(pgi_at28_io[i]->base_addr, pgi_at28_io[i]->index, 1); /* output */

        if (data & (0x1 << i)) {
            gpio_write(pgi_at28_io[i]->base_addr, pgi_at28_io[i]->index, 1);
        } else {
            gpio_write(pgi_at28_io[i]->base_addr, pgi_at28_io[i]->index, 0);
        }
    }

    msleep(1);

    at28_set_oe(1); /* output high-Z */
    at28_set_ce(0);
    at28_set_we(0);

    /* write */
    msleep(4);

    at28_set_ce(0);
    at28_set_we(1);

    /* write finish. */

    /* restore to default (read state) */
    for(i = 0; i < 8; i++) {
        gpio_init(pgi_at28_io[i]->base_addr, pgi_at28_io[i]->index, 0);
    }
#endif
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

uint8_t num2data(uint8_t num)
{
    uint8_t display_data;
    num = num % 10;

    set_bit(&display_data, 7, ssd_table[num].A);
    set_bit(&display_data, 6, ssd_table[num].B);
    set_bit(&display_data, 5, ssd_table[num].C);
    set_bit(&display_data, 4, ssd_table[num].D);
    set_bit(&display_data, 3, ssd_table[num].E);
    set_bit(&display_data, 2, ssd_table[num].F);
    set_bit(&display_data, 1, ssd_table[num].G);
    set_bit(&display_data, 0, ssd_table[num].DP);

    return display_data;

}


int ssd_set(uint8_t dindex, uint8_t num)
{
    uint16_t data;
    data = (dindex << 8) | num2data(num);
    hc595_set_data(data);
    return 0;
}

int thread_ssd_display()
{
    return 0;
}

int main() {

    hc595_init();

    printf("%s\r\n", sys_banner);

    //ssd_set_all();

    while (1) {
        ssd_set(D1, 1);
        ssd_set(D2, 2);
        ssd_set(D3, 3);
        ssd_set(D4, 4);
    }

    while(1);

    return 0;
}
