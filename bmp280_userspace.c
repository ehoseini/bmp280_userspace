#include <errno.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define BMP280_DIG_T1_LSB_ADDR 0x88
#define I2C_CLIENT_ADDRESS 0x77
#define NUMBER_OF_BYTES_LSB 24
#define NUMBER_OF_BYTES_MSB 6
#define SUCCESS 0

int main(void)
{
    int fd;
    int ret;
    int dig_T1, dig_T2, dig_T3;
    int dig_P1, dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
    char *bus = "/dev/i2c-1";
    char data[NUMBER_OF_BYTES_LSB] = {0};
    char reg[1] = {BMP280_DIG_T1_LSB_ADDR};
    char CR[2] = {0};
    double var1, var2, T, t_fine;
    double P;
    long adc_P, adc_T;

    if ((fd = open(bus, O_RDWR)) < 0) {
        printf("Failed to open the bus.\n");
        exit(1); 
    }
    
    ret = ioctl(fd, I2C_SLAVE, I2C_CLIENT_ADDRESS);

    if (ret < 0){
        printf("%s\n", strerror(errno));
        exit(1); 
    }

    ret = write(fd, reg, 1);

    if (ret < 0) {
        printf("%s\n", strerror(errno));
        exit(1);
    }

    ret = read(fd, data, NUMBER_OF_BYTES_LSB);

    if (ret < 0) {
        printf("%s\n", strerror(errno));
        exit(1);
    } else if (ret == NUMBER_OF_BYTES_LSB) {
        ret = SUCCESS;
    } else {
        printf("NUMBER_OF_BYTES_LSB not read\n");
    }

    dig_T1 = (data[1] << 8) + data[0];   
    dig_T2 = (data[3] << 8) + data[2];   
    dig_T3 = (data[5] << 8) + data[4];   
                                   
    dig_P1 = (data[7] << 8) + data[6];   
    dig_P2 = (data[9] << 8) + data[8];   
    dig_P3 = (data[11] << 8) + data[10];
    dig_P4 = (data[13] << 8) + data[12];
    dig_P5 = (data[15] << 8) + data[14];  
    dig_P6 = (data[17] << 8) + data[16];  
    dig_P7 = (data[19] << 8) + data[18];  
    dig_P8 = (data[21] << 8) + data[20];  
    dig_P9 = (data[23] << 8) + data[22];  

    if (dig_P2 > 32767) {
        dig_P2 -= 65536;
    }
    if (dig_P3 > 32767) {
        dig_P3 -= 65536;
    }
    if (dig_P4 > 32767) {
        dig_P4 -= 65536;
    }
    if (dig_P5 > 32767) {
        dig_P5 -= 65536;
    }
    if (dig_P6 > 32767) {
        dig_P6 -= 65536;
    }
    if (dig_P7 > 32767) {
        dig_P7 -= 65536;
    }
    if (dig_P8 > 32767) {
        dig_P8 -= 65536;
    }
    if (dig_P9 > 32767) {
        dig_P9 -= 65536;
    }

    CR[0] = 0xF4;
    CR[1] = 0x27; 

    ret = write(fd, CR, 2); 

    if (ret < 0) {
        printf("%s\n", strerror(errno));
        exit(1);
    }

    CR[0] = 0xF5; 
    CR[1] = 0x00; 

    ret = write(fd, CR, 2);

    if (ret < 0) {
        printf("%s\n", strerror(errno));
        exit(1);
    }

    reg[0] = 0xF7;

    ret = write(fd, reg, 1);

    if (ret < 0) {
        printf("%s\n", strerror(errno));
        exit(1);
    }

    ret = read(fd, data, NUMBER_OF_BYTES_MSB);

    if (ret < 0) {
        printf("%s\n", strerror(errno));
        exit(1);
    } else if (ret == NUMBER_OF_BYTES_MSB) {
        ret = SUCCESS;
    } else {
        printf("NUMBER_OF_BYTES_MSB not read\n");
    }

    adc_P = ((long) (data[2] & 0xF0) + ((long) data[1] << 8) + ((long) data[0] << 16)) / 16;
    adc_T = ((long) (data[5] & 0xF0) + ((long) data[4] << 8) + ((long) data[3] << 16)) / 16;

    var1 = (((double) adc_T) / 16384.0 - ((double) dig_T1) / 1024.0) * ((double) dig_T2); 
    var2 = ((((double) adc_T) / 131072.0 - ((double) dig_T1) / 8192.0) * (((double) adc_T) / 131072.0 - ((double) dig_T1) / 8192.0)) * ((double) dig_T3);
    t_fine = (long) (var1 + var2);
    T = (var1 + var2) / 5120.0;

    var1 = t_fine / 2.0 - 64000.0;
    var2 = (((var1 * var1 * ((double) dig_P6) / 32768.0) + var1 * ((double) dig_P5) * 2.0) / 4.0) + ((double) dig_P4) * 65536.0;
    var1 = (1.0 + (((((double) dig_P3) * var1 * var1 / 524288.0 + ((double) dig_P2) * var1) / 524288.0)) / 32768.0) * ((double) dig_P1);
    P = ((1048576.0 - (double) adc_P) - (var2 / 4096.0)) * 6250.0 / var1;
    var1 = ((double) dig_P9) * P * P / 2147483648.0;
    var2 = P * ((double) dig_P8) / 32768.0;
    P = (P + (var1 + var2 + ((double) dig_P7)) / 16.0) / 100.0;

    printf("Temperature: %.2f [Celsius]\n", T);
    printf("Pressure: %.2f [hPa]\n", P);

    return ret;
}
