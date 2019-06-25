/*
 * i2c.h - send and receive bytes on an I2C bus
 *
 * Copyright 2019 T Glyn Thomas (glyn@tgtsystems.com)
 */

#ifndef _I2C_H_
#define _I2C_H_

// I2C bus
int send_byte(unsigned char i2c_addr, const unsigned char &c);
int send_bytes(unsigned char i2c_addr, unsigned char buff[], int nbytes);
int read_bytes(unsigned char i2c_addr, unsigned char buff[], int nbytes);

#endif /* _I2C_H_ */
