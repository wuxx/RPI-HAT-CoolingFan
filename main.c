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


int main() {

    hc595_init();

    printf("%s\r\n", sys_banner);

    return 0;
}
