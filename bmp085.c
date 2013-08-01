//来源于jiabin924
#include <core.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "Wire.h"
#define BMP085_ADDRESS 0x77  // BMP085的I2C地址
const unsigned char OSS = 3;  // 采样设置


// 校准值
short int ac1;
short int ac2; 
short int ac3; 
unsigned short int ac4;
unsigned short int ac5;
unsigned short int ac6;
short int b1; 
short int b2;
short  mb;
short  mc;
short int md;


// b5用于计算bmp085GetTemperature(...)，它也同时用于bmp085GetPressure(...)
// 所以Temperature(...)必须在Pressure(...)之前声明


int b5; 
short temperature;
float wendu;
long pressure;
const float p0 = 101325;     // Pressure at sea level (Pa)
float altitude;
// Read 2 bytes from the BMP085
// First byte will be from 'address'
// Second byte will be from 'address'+1




short bmp085ReadInt(unsigned char address)
{
//  unsigned char msb, lsb;
  
//  Wire.beginTransmission(BMP085_ADDRESS);
//  Wire.write(address);
//  Wire.endTransmission();
  
//  Wire.requestFrom(BMP085_ADDRESS, 2);
//  while(Wire.available()<2)
//    ;
//  msb = Wire.read();
//  lsb = Wire.read();
  
//  return (int) msb<<8 | lsb;
  unsigned char msb, lsb;
short tmp;
  char i=0;
  char val[10];
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();


  Wire.requestFrom(BMP085_ADDRESS, 2);
 while(Wire.available())
        {
  val[i++] = Wire.read();
}
// return (int) val[0]<<8 | val[1];
// printf("oo1 %d\n",val[0]);
// printf("oo2 %d\n",val[1]);
tmp=((short)val[0] <<8 )|val[1];


 return tmp;
}






// Stores all of the bmp085's calibration values into global variables
// Calibration values are required to calculate temp and pressure
// This function should be called at the beginning of the program
void bmp085Calibration()
{
  ac1 = bmp085ReadInt(0xAA);
  ac2 = bmp085ReadInt(0xAC);
  ac3 = bmp085ReadInt(0xAE);
  ac4 = bmp085ReadInt(0xB0);
  ac5 = bmp085ReadInt(0xB2);
  ac6 = bmp085ReadInt(0xB4);
  b1 = bmp085ReadInt(0xB6);
  b2 = bmp085ReadInt(0xB8);
  mb = bmp085ReadInt(0xBA);
  mc = bmp085ReadInt(0xBC);
  md = bmp085ReadInt(0xBE);
}


// Calculate temperature given ut.
// Value returned will be in units of 0.1 deg C
short bmp085GetTemperature(unsigned int ut)
{
  int x1, x2,temp;
  
  x1 = (((int)ut - (int)ac6)*(int)ac5) >> 15;
  x2 = ((int)mc << 11)/(x1 + md);
  b5 = x1 + x2;
  temp =((b5 + 8)>>4);
//printf("x2:%d mc:%d md %d \n ",x2,mc,md);
//printf("%d ut \n",ut); 
//printf("%d ac6 \n ",ac6);
//printf("%d ac5 \n ",ac5);
  return temp;
}


// Calculate pressure given up
// calibration values must be known
// b5 is also required so bmp085GetTemperature(...) must be called first.
// Value returned will be pressure in units of Pa.
unsigned int bmp085GetPressure(unsigned int up)
{
  int x1, x2, x3, b3, b6, p;
  unsigned int b4, b7;
  
  b6 = b5 - 4000;
  // Calculate B3
  x1 = (b2 * (b6 * b6)>>12)>>11;
  x2 = (ac2 * b6)>>11;
  x3 = x1 + x2;
  b3 = (((((int)ac1)*4 + x3)<<OSS) + 2)>>2;
  
  // Calculate B4
  x1 = (ac3 * b6)>>13;
  x2 = (b1 * ((b6 * b6)>>12))>>16;
  x3 = ((x1 + x2) + 2)>>2;
  b4 = (ac4 * (unsigned int)(x3 + 32768))>>15;
  
  b7 = ((unsigned int)(up - b3) * (50000>>OSS));
  if (b7 < 0x80000000)
    p = (b7<<1)/b4;
  else
    p = (b7/b4)<<1;
    
  x1 = (p>>8) * (p>>8);
  x1 = (x1 * 3038)>>16;
  x2 = (-7357 * p)>>16;
  p+=(x1 + x2 + 3791)>>4;
//  printf("up: %d \n",up);
// printf("x1:%d x2:%d x3:%d b3:%d b6:%d \n",x1,x2,x3,b3,b6); 
  return p;
}
// Read 1 byte from the BMP085 at 'address'
char bmp085Read(unsigned char address)
{
  unsigned char data;
  
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();
  
  Wire.requestFrom(BMP085_ADDRESS, 1);
  while(Wire.available())
    {
    
  return Wire.read();
 }
}


// Read the uncompensated temperature value
unsigned int bmp085ReadUT()
{
  unsigned int ut;
  
  // Write 0x2E into Register 0xF4
  // This requests a temperature reading
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(0xF4);
  Wire.write(0x2E);
  Wire.endTransmission();
  
  // Wait at least 4.5ms
//  delay(5);
  usleep(5000);
  // Read two bytes from registers 0xF6 and 0xF7
  ut = bmp085ReadInt(0xF6);
// printf("dandudu 0xF7 :%d \n",bmp085Read(0xF7));
   return ut;
}


// Read the uncompensated pressure value
unsigned int bmp085ReadUP()
{
 char i=0; 
 unsigned char values [10];
  unsigned int  up = 0;
  
  // Write 0x34+(OSS<<6) into register 0xF4
  // Request a pressure reading w/ oversampling setting
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(0xF4);
  Wire.write(0x34 + (OSS<<6));
  Wire.endTransmission();
  
  // Wait for conversion, delay time dependent on OSS
  usleep((2 + (3<<OSS))*1000);
  
  // Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB)
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(0xF6);
  Wire.endTransmission();
  Wire.requestFrom(BMP085_ADDRESS,3);
  
  // Wait for data to become available
  while(Wire.available())
    {
 //  printf("diaoyongchisu: %d \n",i);
   values[i++]= Wire.read();
  }
// printf("values[0]: %d values[1]:%d values[2]:%d ",values[0],values[1],values[2]);
  up = (((unsigned int) values[0] << 16) | ((unsigned int) values[1] << 8) | (unsigned int) values[2]) >> (8-OSS);
  
  return up;
}


void setup(){
 // Serial.begin(9600);
  Wire.begin();


  bmp085Calibration();
}


void loop()
{
  int temperature = bmp085GetTemperature(bmp085ReadUT()); //MUST be called first
  int pressure = bmp085GetPressure(bmp085ReadUP());
//  float atm = pressure / 101325; // "standard atmosphere"
  float altitude = (float)44330 * (1 - pow(((float) pressure/p0), 0.190295)); //Uncompensated caculation - in Meters 


 printf("Temperature %f C  \n", ((double)temperature)/10);


printf("Pressure: %d \n",pressure);
//  printf("Pressure: ");
 // printf("%f",pressure); //whole number only.
//  printf(" Pa1 \n");


//  printf("Standard Atmosphere: ");
//  printf("%f",atm); //display 4 decimal places
//  printf("\n");
  printf("Altitude: %f \n ",altitude);
//  printf("%f",altitude); //display 2 decimal places
//  printf(" M");


//  printf("\n");//line break


  delay(1000); //wait a second and get values again.
