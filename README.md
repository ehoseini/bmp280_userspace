# BMP280 userspace driver 

## Introduction

An userspace bmp280 device driver for performing readouts of the temperature and pressure. The BMP280 is an absolute barometric pressure sensor designed mainly for mobile applications. The device communicates via the i2c serial protocol.

## Getting started

Connect the sensor to the i2c bus and type the following command:

```c
gcc -o bmp280 bmp280_userspace.c && ./bmp280
```

This will print the temperature and pressure values in Celsius and hPa respectively.

```bash
Temperature: 22.30 [Celsius]
Pressure: 1013.59 [hPa]
```
