//------------------------------------------------------------------------------------
// Base_8051
//------------------------------------------------------------------------------------
//
// AUTH: DF
// DATE: 18-09-2017
// Target: C8051F020
//
// Tool chain: KEIL UV4
//
// Application de base qui configure seulement l'oscillateur et le watchdog
//-------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------------
#include "c8051f020.h" // SFR declarations
#include <intrins.h>  
#include <stdio.h>
#include <stdlib.h>
#include "init.h"

sbit LED = P1^6; // Led verte embarquée sur la carte
sbit TEMP = P1^7; // temp sensor
sbit DEBUG = P1^5; // debug
#ifdef MASTER
sbit NSS = P2^6;
#endif

char res[5] = {0};

void fct_tempo(int duree_us);

void wait(int duree_ms)
{
	int i = 0;
	for(i; i < duree_ms; i++)
	{
		fct_tempo(1000);
	}
}

void rthConfig()
{
	
	P1MDOUT &= ~0x80;
}
	

void rthInit()
{
	DEBUG = 1;
	LED = 1;
	TEMP = 0;
	wait(20);
	TEMP = 1;
	
	fct_tempo(20);

	while(TEMP != 0)
	{
	}
	
	while(TEMP != 1)
	{
	}
	
	while(TEMP != 0)
	{
	}
}

void rthRead()
{
	int i = 0;
	int y = 0;
	char readValue = 0;
	for(i; i < 5; i ++)
	{
		for(y=0; y < 8; y++)
		{
			readValue = TEMP;
			while(TEMP != 1)
			{
			}
			fct_tempo(50);
			readValue = TEMP;
			res[i] = res[i] << 1;
			res[i] += readValue;
			while(TEMP != 0)
			{
			}
		}
		
	}
	
	DEBUG = 0;
	LED = 0;
}

// inline function to swap two numbers
void swap(char *x, char *y) {
	char t = *x; *x = *y; *y = t;
}

// function to reverse buffer[i..j]
char* reverse(char *buffer, int i, int j)
{
	while (i < j)
		swap(&buffer[i++], &buffer[j--]);

	return buffer;
}

// Iterative function to implement itoa() function in C
char* itoa(int value, char* buffer)
{
	// consider absolute value of number
	int n = abs(value);

	int i = 0;
	while (n)
	{
		int r = n % 10;

		if (r >= 10) 
			buffer[i++] = 65 + (r - 10);
		else
			buffer[i++] = 48 + r;

		n = n / 10;
	}

	// if number is 0
	if (i == 0)
		buffer[i++] = '0';

	// If base is 10 and value is negative, the resulting string 
	// is preceded with a minus sign (-)
	// With any other base, value is always considered unsigned
	if (value < 0)
		buffer[i++] = '-';

	buffer[i] = '\0'; // null terminate string

	// reverse the string and return it
	return reverse(buffer, 0, i - 1);
}

void uart_write_c(char c)
{
	while(TI0 != 1);
	TI0 = 0;
	SBUF0 = c;		
}

void uart_write_s(char* s)
{
	int i = 0;
	while(s[i] != '\0')
	{
		uart_write_c(s[i]);
		i++;
	}
}

void emptyBuff(char* buff,int size)
{
	int i = 0;
	while(i < size)
	{
		buff[i] = '\0';
		i++;
	}
}

void uart_sendValue(int value)
{
	char toSend[10];
	emptyBuff(toSend,10);
	itoa(value, toSend);
	uart_write_s(toSend);
}

void uart_sendAll(int x, int y, int z)
{
	//uart_write_s("T:");
	uart_sendValue(res[0]);
	uart_write_s(";");
	//uart_write_s(" H:");
	uart_sendValue(res[2]);
	uart_write_s(";");
	//uart_write_s(" X:");
	uart_sendValue(x);
	uart_write_s(";");
	//uart_write_s(" Y:");
	uart_sendValue(y);
	uart_write_s(";");
	//uart_write_s(" Z:");
	uart_sendValue(z);
	uart_write_s("\n");
}



char spi_transaction_c(char c)
{
	char readValue;
#ifdef MASTER
	SPIF = 0;
	//NSS = 0;
	SPI0DAT = c;
	while(SPIF == 0){}
	readValue = SPI0DAT;
	//NSS = 1;
#else
	while(SPIF == 0){}
	readValue = SPI0DAT;
	SPI0DAT = c;
	SPIF = 0;
#endif	
	return readValue;
}

/*char readRegister(char registerAddress)
{
	char readValue;
	char byteToSend = registerAddress;
	// Set to read
	byteToSend |= 0x80;
	// Set Multiple Byte to 0
	byteToSend &= ~0x40;
	
	NSS = 0;
	// Write the register
	readValue = spi_transaction_c(byteToSend);
	// Write to be able to read the result
	readValue = spi_transaction_c(0xFF);
	NSS = 1;
	return readValue;
}*/

unsigned char* readDataRegisters(unsigned char* dataBuffer)
{
	int i = 0;
	char address = 0x32;
	// Set to read and multiple bytes to 1
	address |= 0xC0;
	NSS = 0;
	// Write the register address
	spi_transaction_c(address);
	// Loop over the 6 data
	for(i = 0; i < 6; i++)
	{
		dataBuffer[i] = spi_transaction_c(0xFF);
	}
	NSS = 1;
	return dataBuffer;
}

void writeRegister(char registerAddress, char value)
{
	char byteToSend = registerAddress;
	// Set to write and set Multiple Byte to 0
	byteToSend &= ~0xC0;
	
	NSS = 0;
	// Write the register
	spi_transaction_c(byteToSend);
	// Write the value
	spi_transaction_c(value);
	NSS = 1;
}

void startADXL()
{
	// DATA_FORMAT
	writeRegister(0x31,0x0B);
	// POWER_CTL
	writeRegister(0x2D,0x08);
	// INT_ENABLE
	writeRegister(0x2E,0x80);
}


//------------------------------------------------------------------------------------
// MAIN Routine
//------------------------------------------------------------------------------------
void main (void)
{
	char readValue;
	unsigned char readData[6];
	int x, y, z;
#ifdef MASTER
	P2MDOUT |= 0x40;
	NSS=1;
#endif
	
	P1MDOUT |= 0x60;
  Init_Device();
	rthConfig();
	DEBUG = 0;
	LED = 0;
	
	wait(5);	
	startADXL();
	
  while (1)
  {
		// GET ADXL
		x = 0;
		y = 0;
		z = 0;
		readDataRegisters(readData);
		x = readData[1];
		x = x << 8;
		x += readData[0];
		
		y = readData[3];
		y = y << 8;
		y += readData[2];
		
		z = readData[5];
		z = z << 8;
		z += readData[4];
		
		// GET TEMP
		emptyBuff(res,5);
		rthInit();
		rthRead();
		
		uart_sendAll(x,y,z);
		
		wait(500);
		
		
	}	 	 
}

 

 