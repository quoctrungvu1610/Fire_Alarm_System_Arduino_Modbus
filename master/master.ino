#include <ModbusRtu.h>
#include <Servo.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>


LiquidCrystal_I2C lcd(0x27,16,2); 
// data array for modbus network sharing
uint16_t au16data[10];
uint8_t u8state;
Servo myservo;   
int pos = 0;    
#define TXEN  8 

/**
*  Modbus object declaration
*  u8id : node id = 0 for master, = 1..247 for slave
*  u8serno : serial port (use 0 for Serial)
*  u8txenpin : 0 for RS-232 and USB-FTDI 
*               or any pin number > 1 for RS-485
*/
Modbus master(0,0,TXEN); // this is master and RS-232 or USB-FTDI

/**
* This is an structe which contains a query to an slave device
*/
modbus_t telegram;
unsigned long u32wait;

void setup() 
{
    master.begin( 19200 ); // baud-rate at 19200
    master.setTimeOut( 2000 ); // if there is no answer in 2000 ms, roll over
    u32wait = millis() + 1000;
    u8state = 0;
    myservo.attach(10);
    lcd.init();                    
    lcd.backlight(); 
}

void loop() 
{
    switch( u8state ) 
    {
        case 0: 
            if (millis() > u32wait) u8state++; // wait state
        break;
        case 1: 
            telegram.u8id = 1; // slave address
            telegram.u8fct = 3; // function code (this one is registers read)
            telegram.u16RegAdd = 0; // start address in slave
            telegram.u16CoilsNo = 4; // number of elements (coils or registers) to read
            telegram.au16reg = au16data; // pointer to a memory array in the Arduino
            master.query( telegram ); // send query (only once)
            u8state++;
        break;
        case 2:
            master.poll(); // check incoming messages
            u8state = 0;
            u32wait = millis() + 100; 
            for(int i = 0; i < 4; i++)
            {
                Serial.println(au16data[i]);
            }
            if(au16data[1]>25)
            {
                digitalWrite(9,HIGH);
                delay(10);
                digitalWrite(9,LOW);
            }
            else
            {
            lcd.setCursor(0,0);
            lcd.print("Hump:");
            lcd.setCursor(5,0);
            lcd.print(au16data[2]);
            lcd.setCursor(7,0);
            lcd.print("%");
            lcd.setCursor(9,0);
            lcd.print("Temp:");
            lcd.setCursor(14,0);
            lcd.print(au16data[3]);
            
            lcd.setCursor(0,1);
            lcd.print("MQ-2:");
            lcd.setCursor(5,1);
            lcd.print(au16data[1]);
            lcd.setCursor(9,1);
            lcd.print("Flme:");
            lcd.setCursor(14,1);
            lcd.print(au16data[0]);
        }
        break;
    }
}
