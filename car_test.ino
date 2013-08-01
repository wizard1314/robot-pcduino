#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<ctype.h>
#include<errno.h>
#define PORT 5000

#include <core.h>
#define  servo_x      4
#define  servo_y     7
#define  cycle                 20000
#define A(x) (struct sockaddr*)(&x)
 
int led_pin = 2;
int listenfd = 0, connfd = 0;
int n;
struct sockaddr_in serv_addr; 
char sendBuff[1025];
time_t ticks; 

#define pinI1     8  //define IN1 interface
#define pinI2     11 //define IN2 interface 
#define speedpinA 9  //enable motor A
#define pinI3     12 //define IN3 interface 
#define pinI4     13 //define IN4 interface 
#define speedpinB 10 //enable motor B
#define spead     14//define the spead of motor
void forward()
{
     analogWrite(speedpinA,spead);//input a simulation value to set the speed
     analogWrite(speedpinB,spead);
     digitalWrite(pinI4,HIGH);//turn DC Motor B move clockwise
     digitalWrite(pinI3,LOW);
     digitalWrite(pinI2,LOW);//turn DC Motor A move anticlockwise
     digitalWrite(pinI1,HIGH);
}
void backward()
{
     analogWrite(speedpinA,spead);//input a simulation value to set the speed
     analogWrite(speedpinB,spead);
     digitalWrite(pinI4,LOW);//turn DC Motor B move anticlockwise
     digitalWrite(pinI3,HIGH);
     digitalWrite(pinI2,HIGH);//turn DC Motor A move clockwise
     digitalWrite(pinI1,LOW);
}
void left()
{
     analogWrite(speedpinA,spead);//input a simulation value to set the speed
     analogWrite(speedpinB,spead);
     digitalWrite(pinI4,HIGH);//turn DC Motor B move clockwise
     digitalWrite(pinI3,LOW);
     digitalWrite(pinI2,HIGH);//turn DC Motor A move clockwise
     digitalWrite(pinI1,LOW);
}
void right()
{
     analogWrite(speedpinA,spead);//input a simulation value to set the speed
     analogWrite(speedpinB,spead);
     digitalWrite(pinI4,LOW);//turn DC Motor B move anticlockwise
     digitalWrite(pinI3,HIGH);
     digitalWrite(pinI2,LOW);//turn DC Motor A move clockwise
     digitalWrite(pinI1,HIGH);
}
void stop()
{
     digitalWrite(speedpinA,0);// Unenble the pin, to stop the motor. this          should be done to avid damaging the motor. 
     digitalWrite(speedpinB,0);
     digitalWrite(pinI4,LOW);//turn DC Motor B move anticlockwise
     digitalWrite(pinI3,LOW);
     digitalWrite(pinI2,LOW);//turn DC Motor A move clockwise
     digitalWrite(pinI1,LOW);
}

void servo_contor(char servo_pin,int degree)
{
     //  for(int i=0;i<3;i++)
     //{
       digitalWrite(servo_pin,HIGH);
       delayMicroseconds(degree);
       digitalWrite(servo_pin,LOW);
       delayMicroseconds(cycle - degree);
     //}
}

void port_init()
{
   pinMode(servo_x,OUTPUT);
   pinMode(servo_y,OUTPUT);

   pinMode(pinI1,OUTPUT);
   pinMode(pinI2,OUTPUT);
   pinMode(speedpinA,OUTPUT);
   pinMode(pinI3,OUTPUT);
   pinMode(pinI4,OUTPUT);
   pinMode(speedpinB,OUTPUT);

   servo_contor(4,1500);
   servo_contor(7,1500);
   delay(100);
}

void* func(void*p)
{
        static int x=1500;
        static int y=1500;
  int c = (int)p;
	char buf[1000];
	for(;;)
	{
	int n = 	read(c,buf,sizeof(buf)-1);
	if(n<=0) break;
	buf[n] = '\0';	
	int i;
	write(1,buf,n);
	//digitalWrite(servo_x,LOW);
	//digitalWrite(servo_y,LOW);
	servo_contor(4,x);
	servo_contor(7,y);
        switch(buf[0])
        {
           case 'o' :          digitalWrite(led_pin, HIGH);  break;          // set the LED on
           case 'f':            digitalWrite(led_pin,LOW);    break;  // set the LED off
           case 'w':          forward();break;
           case 's':           backward() ; break;
           case 'a':           left();break;
           case 'd':           right();break;
           case 'i':             if(x<2500) x+=25; break;
           case 'k':            if(x>500)   x-=25;  break;
           case 'j':             if(y<2500) y+=25; break;
           case 'l':             if(y>500)   y-=25; break;
        }
     //     if( (buf[0]=='i') || (buf[0]=='k')  )
               // servo_contor(4,x);
        //  if( (buf[0]=='j') || (buf[0]=='l') )
               // servo_contor(7,y);
          if( (buf[0]=='w') || (buf[0]=='s') || (buf[0]=='a') || (buf[0]=='d') )
               delay(50);           
        stop();
	}
	close(c);
	
}

int s;
struct sockaddr_in si;  
void setup()
{    
         pinMode(2,OUTPUT);
	s = socket(AF_INET,SOCK_STREAM,0);		
	si.sin_family = AF_INET;
	si.sin_addr.s_addr =  INADDR_ANY;
	si.sin_port = htons(PORT);
	if(bind(s,A(si),sizeof(si)))
	{
		printf("%s\n",strerror(errno)); exit(1);
	}
        listen(s,5);
}


void loop()
{
                port_init();
    		socklen_t len = sizeof(si);
		int c = accept(s,A(si),&len);
		while(c<0);
		//welcome friend form ip,port
                char buf[100],ip[100];
		inet_ntop(si.sin_family,&si.sin_addr,ip,sizeof(ip));
		pthread_t id;
		pthread_create(&id,NULL,func,(void*)c);
		pthread_detach(id);
}
