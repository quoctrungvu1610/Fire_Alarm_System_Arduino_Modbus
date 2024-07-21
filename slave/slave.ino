#include "DHT.h"
#include <Servo.h>
#include <Wire.h> 
#include <ModbusRtu.h>
#include <math.h>


#define ID   1
#define TXEN  8



const int DHTPIN = 2;
const int DHTTYPE = DHT11;
DHT dht(DHTPIN, DHTTYPE);
Servo myservo;
int pos = 0;  
Modbus slave(ID, 0, TXEN); // this is slave ID and RS-232 or USB-FTDI
boolean led;
int8_t state = 0;
unsigned long tempus;
float hump;
float temp;
int flameAnalog;
float gasAnalog;
float flameVoltage;
float gasVoltage;
float R0;
float RS;
float smokePpm;
// data array for modbus network sharing
uint16_t au16data[9];


void io_setup() 
{
  // define i/o
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(13, OUTPUT);
  myservo.attach(9);
}


void io_poll()
{
    //read analog inputs
    au16data[0] = flameAnalog;
    //Serial.println(au16data[0]);
    au16data[1] = smokePpm;
    au16data[2] = hump;
    au16data[3] = temp;
    if(au16data[1]>25)
    {
        digitalWrite(11,HIGH);
        for(pos = 0; pos < 180; pos += 1)
        {
            myservo.write(pos);
            delay(15);
        }
        for(pos = 180; pos>=1; pos-=1) 
        {                           
            myservo.write(pos);
            delay(15);
        } 
        digitalWrite(11,LOW); 
    }
}

void setup()
{
    io_setup(); // I/O settings
    
    // start communication
    slave.begin( 19200 );
    tempus = millis() + 100;
    digitalWrite(13, HIGH );
    pinMode(11,OUTPUT); 
}

void loop() 
{
    // poll messages
    // blink led pin on each valid message
    hump = dht.readHumidity();
    temp = dht.readTemperature();
    gasAnalog = analogRead(A1);
    gasVoltage = gasAnalog*5.0/1023;
    RS = (5.0-gasVoltage)/gasVoltage;
    R0 = 0.47;
    smokePpm = pow(10.0,(log10(RS/R0)-1.617856412)/-0.44340257);
    flameAnalog = analogRead(A0);
    state = slave.poll(au16data,9);
    if (state != 0) 
    {
        digitalWrite(6,LOW);
        delay(500);
        digitalWrite(6,HIGH);
    } 
    if (state > 4) 
    {
        tempus = millis() + 50;
        digitalWrite(13, HIGH);
    }
    if (millis() > tempus)
    {
        digitalWrite(13, LOW ); 
    }
    io_poll();
} 