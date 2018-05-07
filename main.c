
/*
 * GPS-UART-BUF.c
 *
 * Created: 10/13/2017 5:37:46 PM
 * Author : madiv
 */ 
#define F_CPU 1000000UL 
#include <string.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "sms.h"

char byteGPS=-1;
char linea[100] = "";
char comandoRMC[7] = "$GPRMC";
char comandoGGA[7] = "$GPGGA";
int cont=0;
int bien=0;
int bien1=0;
int conta=0; 
char *RMC;
char *GGA;
int GPS_position_count=0;
int datacount=0;
int datacount1=0;
int y=0;
int x=0;

#define FOSC 2000000UL // Clock Speed
#define BAUD 9600
#define MYUBRR (((FOSC / (BAUD * 16UL))) - 1)

//static FILE uart0_output = FDEV_SETUP_STREAM(USART0_Transmit, NULL, _FDEV_SETUP_WRITE);
static FILE uart1_output = FDEV_SETUP_STREAM(USART1_Transmit, NULL, _FDEV_SETUP_WRITE);
static FILE uart1_input = FDEV_SETUP_STREAM(NULL, USART1_Receive, _FDEV_SETUP_READ);
//static FILE uart0_input = FDEV_SETUP_STREAM(NULL, USART0_Receive, _FDEV_SETUP_READ);

int CheckSMS();
void checkOKstatus();
int sample_GPS_data (char *GPS0, char *GPS1);
int grabGPS();
int CarStatus(int p);
int sender();
int CompareNumber();
int initialstatus();
int PrintSender();

#define CAR_ON	PORTB |= (1<<PORTB1)
#define CAR_OFF	PORTB &= ~(1<<PORTB1)

int a; int i; char w; int y;
char input;
char buff[20];
char company[]	= "+254700000000";//" //moha's No#
char company2[]	= "+254700000000";//; //fatah's no#
char owner[]	= "+254XXXXXXXXX"; //kevin's no#

int main( void )
{
	
	CAR_OFF;
	
	USART1_Init(MYUBRR);
	USART0_Init(MYUBRR);
	DDRB |= (1<<DDB1); //set PORTB1 as output
	
//  	stdin = &uart0_input;
//  	stdout = &uart1_output; //changed to TX1 for GSM communication. TX0 on Atmega SMD isnt working
//	sei();
//		fdev_close();
		stdout = &uart1_output;
		stdin = &uart1_input;	
	
	_delay_ms(13000);
//	initialstatus();//........................................................................................................................................................return this to function
	while(1) 
	{
		//check availability of SMS
//		CheckSMS(); //check if available unread SMS and its content ........................................................................................................
		grabGPS();
			
//		fdev_close();
// 		stdout = &uart1_output;........................................................................................................
// 		stdin = &uart0_input;........................................................................................................
		_delay_ms(4000);
	}
	return 0;
}

int CheckSMS()
{
	int z = 0; //char w;
	y=0;
	a=0;
	printf("AT\r\n");
	_delay_ms(2000);
//	checkOKstatus();
	
	printf("AT+CMGF=1\r\n");
	_delay_ms(2000);
	//	checkOKstatus();
	
	printf("AT+CMGL=\"REC UNREAD\"\r\n");
	while (a < 2) //skip the <LF>
	{
		w = getchar();
		if (w==0x0A)
		{ a++; }
		else
		{}
	}
	w = getchar();
	
	if (w==0x02B) // if w = +
	{
		sender();
		w = getchar();
		while (w !=  0x0A) //w is not <LF>
		{ w = getchar();}
		
		w = getchar();
		if (w == 0x030)//w is '0'
		{	
			CompareNumber();
			z = buff[14] + buff[15] + buff[16]; //sum values of the 3 buffer values
			if (z < 3) //A scenario of receiving text from an authorized no# with '1' or '0'
				{	buff[13] = 1;
					CAR_OFF; 
				}
			else //A scenario of receiving text from Unauthorized no#
			{buff[13] = 0; }
			  
		}
		else if(w == 0x031)//w is '1'
		{ 
			CompareNumber();
			z = buff[14] + buff[15] + buff[16]; //sum values of the 3 buffer values
			if (z < 3) //A scenario of receiving text from an authorized no# with '1' or '0'
			{	buff[13] = 2;
				CAR_ON;
			}
			else //A scenario of receiving text from Unauthorized no#
			{buff[13] = 0; }  
		}
		else{buff[13] = 6; buff[14] = buff[15] = buff[16] = 0; }
	}
	else if(w==0x04F) // if w = 'O'
	{
		w = getchar();
		if (w==0x04B) // if w = 'K'
		{	buff[13] = 3; buff[14] = buff[15] = buff[16] = 0; 
			initialstatus();
		}
		else
		{ buff[13] = 4; buff[14] = buff[15] = buff[16] = 0; }
		
	}
	else
	{buff[13] = 5; buff[14] = buff[15] = buff[16] = 0; }
		
	int E = buff[13];
		if (E==1) // clear sms storage area if 0/1 is received
		{
//			printf("AT+CMGF=1\r\n");
//			checkOKstatus();
			printf("AT+CMGD=1,4\r\n"); //clearing all SMS in storage AREA
			_delay_ms(2000);
		//	checkOKstatus();
			printf("AT+CMGW=\"");
			PrintSender();
			printf("\",145,\"STO UNSENT\"\r\n");
			_delay_ms(2000);
			printf("0");
			putchar(0x1A); //putting AT-MSG termination CTRL+Z in USART0
		}
		else if (E==2) // clear sms storage area if 0/1 is received
		{
//			printf("AT+CMGF=1\r\n");
//			checkOKstatus();
			printf("AT+CMGD=1,4\r\n"); //clearing all SMS in storage AREA
			_delay_ms(2000);
		//	checkOKstatus();
			printf("AT+CMGW=\"");
			PrintSender();
			printf("\",145,\"STO UNSENT\"\r\n");
			_delay_ms(2000);
			printf("1");
			putchar(0x1A); //putting AT-MSG termination CTRL+Z in USART0
		}
		else
		{}

	return *buff;
}

void checkOKstatus()
{
		w = getchar();
		while (w!=0x04F) // if w = O
		{	w = getchar();	}
			
		while (w!=0x04B) // if w = K
		{	w = getchar();	}
}

int CarStatus(int p)
{
	int c;
	c = p;
	if (c == 1)
	{ CAR_OFF; }
	else if (c == 2)
	{ CAR_ON; }
	else
	{}
	return 0;
}

int sender()
{
	int n;
	w = getchar();
	while (w != 0x02B) // while w is not +
	{ w = getchar(); }
	
	for (n=0; n<13; n++) //capture 13 digit phone number
	{	buff[n] = w;
	w = getchar();}
	
	return *buff;
}

int PrintSender()
{
	int n;
	for (n=1; n<13; n++) //capture 13 digit phone number
	{	printf("%c", buff[n]);}
	return 0;
}

int CompareNumber()
{
	int j;
	buff[14]=buff[15]=buff[16]=0;
	
	for (j=0; j<13; j++)
	{
		if (buff[j]!=company[j])
		{ buff[14] = 1;}
		else{}
		if (buff[j]!=owner[j])
		{ buff[15] = 1;}
		else{}
		if (buff[j]!=company2[j])
		{ buff[16] = 1;}
		else{}
	}
	return *buff;
}

int sample_GPS_data (char *GPS0, char *GPS1)
{

	//int i=0;
	printf("AT\r\n");
	_delay_ms(1000);
	printf("AT+CGATT=1\r\n");
	_delay_ms(2000);
	printf("AT+CIPMUX=0\r\n");
	_delay_ms(1000);
	printf("AT+CSTT=\"APN\",\"\",\"\"\r\n");
	_delay_ms(2000);
	printf("AT+CIICR\n");
	printf("\r\n");
	_delay_ms(3000);
	printf("AT+CIFSR\r\n");
	_delay_ms(2000);
	printf("AT+CIPSTART=\"TCP\",\"IP\",\"PORT\"\r\n");
	_delay_ms(2000);
	printf("AT+CIPSEND\r\n");
	_delay_ms(1000);

	printf("#CAR PLATE NO:[KCQ 450R]\r\r%s\r%s\r\nBUFF[13] = 1\r\r\nCar status is = 1\r\r\nText no# is :0727XXXXXX\r\r\n",GPS0,GPS1);
	_delay_ms(2500);
	
// 	printf("BUFF[13] = %d\r\n", buff[13]);
// 	_delay_ms(200);
// 	printf("Car status is = %d\r\nText no# is :", buff[13]);
// 	PrintSender();
// 	printf("\r\n");
	putchar(0x1A); //putting AT-MSG termination CTRL+Z in USART0
	_delay_ms(2000);
	printf("AT+CIPCLOSE\r\n");
	_delay_ms(2000);
	printf("AT+CIPSHUT\r\n");
	_delay_ms(3000);
	return 0;
}

int initialstatus()
{
	//char w;
	y=0;
	a=0;
	printf("AT\r\n");
	_delay_ms(2000);
	
	printf("AT+CMGF=1\r\n");
	_delay_ms(2000);
	
	printf("AT+CPMS=\"MT\",\"SM\",\"ME\"\r\n");
	_delay_ms(2000);
	
	printf("AT+CMGR=1\r\n");
	while (a < 2) //skip the <LF>
	{
		w = getchar();
		if (w==0x0A)
		{ a++; }
		else
		{}
	}
	w = getchar();
	
	if (w==0x02B) // if w = +
	{
		sender();
		
		w = getchar();
		while (w !=  0x0A) //w is not <LF>
		{ w = getchar();}
		
		w = getchar();
		if (w == 0x030)//w is '0'
		{ buff[13] = 1; _delay_ms(200); CAR_OFF; }
		else if(w == 0x031)//w is '1'
		{ buff[13] = 2; _delay_ms(200); CAR_ON;  }
		else{
			buff[13] = 6;
// 			printf("AT+CMGD=1,4\r\n"); //clearing all SMS in storage AREA
// 			_delay_ms(2000);
			}
	}
	
	else
	{
// 		printf("AT+CMGD=1,4\r\n"); //clearing all SMS in storage AREA
// 		_delay_ms(2000);
	}

	return *buff;
}


int grabGPS()
{
y=0; x=0;
while(x==0 || y==0)
{
	byteGPS=getchar();
	// Read a byte of the serial port
	if (byteGPS == -1)
	{  /* See if the port is empty yet*/ }
	else
	{
		// note: there is a potential buffer overflow here!
		linea[conta]=byteGPS;        // If there is serial port data, it is put in the buffer
		conta++;
		datacount++;
		datacount1++;
		
		//Serial.print(byteGPS);    //If you delete '//', you will get the all GPS information
		
		if (byteGPS==13)
		{
			// If the received byte is = to 13, end of transmission
			// note: the actual end of transmission is <CR><LF> (i.e. 0x13 0x10)
			cont=0;
			bien=0;
			bien1=0;
			// The following for loop starts at 1, because this code is clowny and the first byte is the <LF> (0x10) from the previous transmission.
			for (int i=1;i<7;i++)     // Verifies if the received command starts with $GPR
			{
				if (linea[i]==comandoGGA[i-1])
				{bien++;}
			}
			for (int i=1;i<7;i++)     // Verifies if the received command starts with $GPR
			{
				if (linea[i]==comandoRMC[i-1])
				{bien1++;}
			}
			
			//-----------------------------------------------------------------------
			if(bien==6) // If initial characters match "+GPSRD" string, process the data
			{
				linea[0]='\n';
				GGA = (char*) malloc(datacount+1);
				strcpy(GGA,linea);
				y=1;
				// Serial.print("\r\nGPSRD character count is: ");
				// Serial.println(SRD.length());
			}
			if(bien1==6) // If initial characters match "+GPSRD" string, process the data
			{
				linea[0]='\n';
				RMC = (char*) malloc(datacount1+1);
				strcpy(RMC,linea);
				x=1;
				//Serial.print("\r\nGPRMC character count is: ");
				//Serial.println(RMC.length());
			}

			if (x==1 && y==1)
			{
				//sendData("AT+GPSRD=0",2000,DEBUG);
				_delay_ms(500);
				sample_GPS_data(GGA,RMC);
// 				RMC="";
				free(GGA);
// 				GGA="";
				free(RMC);
// 				x=0;
// 				y=0;
				_delay_ms(2000);
				//sendData("AT+GPSRD=1",2000,DEBUG);
			}
			else{}
			//-----------------------------------------------------------------------
			conta=0;
			datacount=0;
			datacount1=0;
			// Reset the buffer
			for (int i=0;i<100;i++)
			{    //
				linea[i]='\0';
			}
		} //byteGPS==13
	}  //else byteGPS is not null
} //Serial1.available
return 0;
} //testGPS
