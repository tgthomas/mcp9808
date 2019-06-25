/*
 * i2c.cpp - send and recieve bytes on an I2C bus
 *
 * Copyright 2019 T Glyn Thomas (glyn@tgtsystems.com)
 *
 */

#include <stdio.h>
//#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
//#include <errno.h>
//#include <string.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include "i2c.h"

static const char fname[] = "/dev/i2c-1";	// I2C device path
static int fd = 0;				    // I2C file descriptor
static pthread_mutex_t mutex_i2c;   // IMU mutex, serialize access to I2C bus


// send_byte - send a single byte to a device
int send_byte(unsigned char i2c_addr, unsigned char c)
{
    // open device file on first use
    if (fd == 0) {
        fd = open(fname, O_RDWR);
    }
    if (fd == 0) {
        printf("Error: I2C send_byte: can't open file %s\n", fname);
        return(1);
    }

    pthread_mutex_lock(&mutex_i2c);

    // select device
    ioctl(fd, I2C_SLAVE, i2c_addr);

    // send one byte, range [0,255]
    //printf("address %x sending: %d \n", i2c_addr, c);
    int rc = write(fd, &c, 1);

    pthread_mutex_unlock(&mutex_i2c);

    if (rc != 1) {
        printf("Error: I2C send_byte: can't write data to %x\n", i2c_addr);
        return(1);
    }
    return(0);
}


// send_bytes - send bytes to a device
int send_bytes(unsigned char i2c_addr, unsigned char buff[], int nbytes)
{
    // open device file on first use
    if (fd == 0) {
        fd = open(fname, O_RDWR);
    }
    if (fd == 0) {
        printf("Error: I2C send_bytes: can't open file %s\n", fname);
        return(1);
    }

    pthread_mutex_lock(&mutex_i2c);

    // select device
    ioctl(fd, I2C_SLAVE, i2c_addr);

    // send one byte, range [0,255]
    //printf("address %x sending: %d \n", i2c_addr, c);
    int rc = write(fd, buff, nbytes);

    pthread_mutex_unlock(&mutex_i2c);

    if (rc != nbytes) {
        printf("Error: I2C send_bytes: can't write data to %x\n", i2c_addr);
        return(1);
    }
    return(0);
}


// read_bytes - read bytes from a device
int read_bytes(unsigned char i2c_addr, unsigned char buff[], int nbytes)
{
    // open device file on first use
    if (fd == 0) {
        fd = open(fname, O_RDWR);
    }
    if (fd == 0) {
        printf("Error: send_byte: can't open file %s\n", fname);
        return(1);
    }

    pthread_mutex_lock(&mutex_i2c);

    // select device
    ioctl(fd, I2C_SLAVE, i2c_addr);

    // read bytes
    int rc = read(fd, buff, nbytes);

    pthread_mutex_unlock(&mutex_i2c);

    if (rc != nbytes) {
        printf("Error: read_bytes: can't read data from %x\n", i2c_addr);
        return(1);
    }
    return (rc);
}


#ifdef TEST_MOTOR
int main(int argc, char *argv[])
{
    int i2c_addr = 0x2c;

    for (int i = 20; i < 255; i++) {
        unsigned char c = i;
        send_byte(i2c_addr, c);
        usleep(10000);
    }

    for (int i = 255; i >= 20; i--) {
        unsigned char c =  i;
        send_byte(i2c_addr, c);
        usleep(10000);
    }
    return (0);
}
#endif /* TEST_MOTOR */

