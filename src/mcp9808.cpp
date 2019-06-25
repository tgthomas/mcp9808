/*
 * mcp9808.cpp - communicate with an MCP9808 temperature sensor
 *
 * Copyright 2019 T Glyn Thomas (glyn@tgtsystems.com)
 */

#include <iostream>
#include <string>
#include <cstdint>
#include <unistd.h>
#include <cstdio>
#include <chrono>

#include "i2c.h"


// registers, bit 3:0 pointer bits, bits 7:4 are zero.
constexpr unsigned char MCP9808_REG_CONFIG = 0x01;          // configuration
constexpr unsigned char MCP9808_REG_TUPPER = 0x02;          // lower temperature
constexpr unsigned char MCP9808_REG_TLOWER = 0x03;          // upper temperature
constexpr unsigned char MCP9808_REG_TCRIT = 0x04;           // critical temperature
constexpr unsigned char MCP9808_REG_TAMBIENT = 0x05;        // ambiant temperature
constexpr unsigned char MCP9808_REG_MANU_ID = 0x06;         // manufacturer ID
constexpr unsigned char MCP9808_REG_DEV_ID_REV = 0x07;      // device ID and revision
constexpr unsigned char MCP9808_REG_RESOLUTION = 0x08;      // resolution

// config register bit masks
constexpr unsigned int CONFIG_THIST = 0x03 << 9;        // limit hysteresys bytes: 0=0.0, 1=1.5,2=3.0,33=6.0 degC
constexpr unsigned int CONFIG_SHDN = 0x01 << 8;         // shutdown: 0=continuous,1=shutdown/low-power
constexpr unsigned int CONFIG_CRIT_LOCK = 0x01 << 7;    // crit lock: 0=unlocked (default) Tc, 1=locked Tc
constexpr unsigned int CONFIG_WIN_LOCK = 0x01 << 6;     // window lock: 0=unlocked (default) Tu and Tl, 1=locked Tu & Tl
constexpr unsigned int CONFIG_INT_CLEAR = 0x01 << 5;    // interrupt clear: 0=no effect (default), 1=clear interrupt out
constexpr unsigned int CONFIG_ALERT_STAT = 0x01 << 4;   // alert output status: not asserted (default), 1=asserted
constexpr unsigned int CONFIG_ALERT_CTRL = 0x01 << 3;   // alert output control; 0=disabled (default), 1=enabled
constexpr unsigned int CONFIG_ALERT_SEL = 0x01 << 2;    // alert output select: 0=output Tu, Tl & Tc (default), 1=Ta>Tc 
constexpr unsigned int CONFIG_ALERT_POL = 0x01 << 1;    // alert putput polarity: 0=active-low(fefault), 1=active-high
constexpr unsigned int CONFIG_ALERT_MOD = 0x01;         // alert output mode: 0=comp output (default), 1=interupt out

// ambient temperature bit masks
constexpr unsigned int TAMBIENT_TCRIT_BIT = (0x01 << 15);   // ta vs Tc: 0=(Ta < Tc), 1=(Ta >= Tc)
constexpr unsigned int TAMBIENT_TUPPER_BIT = (0x01 << 14);  // Ta vs Tu: 0=(Ta <= Tu), 1=(Ta > tu)
constexpr unsigned int TAMBIENT_TLOWER_BIT = (0x01 << 13);  // Ta vs Tl: 0=(Ta >= Tl), 1=(Ta < Tl)
constexpr unsigned int TAMBIENT_SIGN_BIT = (0x01 << 12);    // sign bit: 0=(Ta >= 0), 1=(Ta < 0)

// resolution 
constexpr unsigned char RES_5 = 0x01;                       // 0.5 degC
constexpr unsigned char RES_25 = 0x01;                      // 0.25 degC
constexpr unsigned char RES_125 = 0x02;                     // 0.125 degC
constexpr unsigned char RES_0625 = 0x03;                    // 0.0625 deg C

/*
 * MCP9808 onversion times
 * 
 * resolution tconv Rate
 *  degC      ms    Samp/s
 * -----------------------
 *  0.5       30    33
 *  0.25      65    15
 *  0.125    130     7
 *  0.0625   250     4
 * -----------------------
 */

// conversion times [microseconds], index res=[0x00,...,0x03]
constexpr unsigned int conv_time[] = { 30000, 65000, 130000, 250000 };
constexpr unsigned int conv_rate[] = { 33, 15, 7, 4 };

// address select pins, VDD=1, GND=0
unsigned char a0 = 0x00;
unsigned char a1 = 0x00;
unsigned char a2 = 0x00;
unsigned char addr_base = 0x18;

// device I2C address
uint8_t i2c_addr = 0x18 | (a2 << 2) | (a1 << 1) | a0;


// sign_extend32 - sign extend a 32-bit value using specified bit as sign-bit
// from linux kernel bitops.h
static inline int32_t sign_extend32(uint32_t value, int index)
{
    uint8_t shift = 31 - index;
    return (int32_t)(value << shift) >> shift;
}


void set_resolution(unsigned char res)
{
    res = res & 0x03;           // use only bits 1:0
    unsigned char data[] = { MCP9808_REG_RESOLUTION, res };

    send_bytes(i2c_addr, data, sizeof(data));

    return;
}


// read the temperature in degC
double read_temp()
{
    // select temperature register
    send_byte(i2c_addr, MCP9808_REG_TAMBIENT);

    // read two-bytes
    unsigned char data[] = { 0x00, 0x00 };
    read_bytes(i2c_addr, data, 2);

    unsigned int reg = (data[0] << 8) | data[1];

    //bool tcrit_flg = reg & TAMBIENT_TCRIT_BIT;
    //bool tupper_flg = reg & TAMBIENT_TUPPER_BIT;
    //bool tlower_flg = reg & TAMBIENT_TLOWER_BIT;
    //bool sign_bit = reg & TAMBIENT_SIGN_BIT;

    //std::cout << "tcrit_flag = " << tcrit_flg << std::endl;
    //std::cout << "tupper_flag = " << tupper_flg << std::endl;
    //std::cout << "tlower_flag = " << tlower_flg << std::endl;
    //std::cout << "sign_bit = " << sign_bit << std::endl;

    int32_t result = (reg & 0x1fff);

    //std::cout << "unsigned result = " << result << std::endl;
    //std::cout << "temperature = " << result*(1.0/16) << std::endl;

    return sign_extend32(result, 12) * (1.0/16);
}





int main() {

    //std::cout << "i2c address = " << i2c_addr << std::endl;
    printf("i2c address = 0x%x\n", i2c_addr);

    unsigned char res = 0x03;
    set_resolution(res);

    int n_ave = 1;              // average over n seconds
    while (true) {
        double temp = 0;
        int nsamp = n_ave*conv_rate[res];
        for (int i = 0; i < nsamp; i++) {
            temp += read_temp();
            usleep(conv_time[res] + 5000);
        }
        temp /= nsamp;
        printf("%3.2f\n", temp);
    }

    return 0;
}
