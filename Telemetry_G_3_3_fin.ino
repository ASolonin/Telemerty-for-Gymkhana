

/*--------------------------------------------------------------------------------------
 Telemetry for gimkhana. It is created on a 512 LED monochrome matrix display
   panel arranged in a 32 x 16 layout. 
 * Libraries are used in this sketch: SPI.h must be included as DMD is written by SPI (the IDE complains otherwise),
DMD.h, TimerOne.h,SystemFont5x7.h, Arial_black_16.h

 red laser ky-008 650 nm as laser sight
 "time of flight" distance meter dVL53L0X

 Copyright (C) 2020  Alexander Solonin (info on my youtube channel)*/
/*--------------------------------------------------------------------------------------
 Includes*/
#include <Arial14.h>
#include <Arial_Black_16_ISO_8859_1.h>
#include <DMD.h>
#include <SystemFont5x7.h> 
#include <SPI.h>        //SPI.h must be included as DMD is written by SPI (the IDE complains otherwise)
#include <DMD.h>    
#include <TimerOne.h>   //
#include <SystemFont5x7.h>
#include <Arial_black_16.h>
#include <Wire.h>
#include <VL53L0X.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>


//Fire up the DMD library as dmd
#define DISPLAYS_ACROSS 1
#define DISPLAYS_DOWN 1
#define LASER 2 // Pin2 is named Laser 
DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);
VL53L0X sensor;
//#define HIGH_SPEED
#define LONG_RANGE
//#define HIGH_ACCURACY
float att; //rate distance change
int lap = 0; //start's counter
String strLap;
char charLap[5];
int distance; //current distance
int distanceZero;//previous distance
bool flagDraw;//draw current time or not
unsigned long timeZero;
unsigned long _time;

/*--------------------------------------------------------------------------------------
  Interrupt handler for Timer1 (TimerOne) driven DMD refresh scanning, this gets
  called at the period set in Timer1.initialize();
--------------------------------------------------------------------------------------*/
void ScanDMD()
{ 
  dmd.scanDisplayBySPI();
}

/*--------------------------------------------------------------------------------------
  setup
  Called by the Arduino architecture before the main loop begins
--------------------------------------------------------------------------------------*/



void setup() {
   pinMode(LASER, OUTPUT); // inicialization Pin10 as exit
  //initialize TimerOne's interrupt/CPU usage used to scan and refresh the display
   Timer1.initialize( 5000 );           //period in microseconds to call ScanDMD. Anything longer than 5000 (5ms) and you can see flicker.
   Timer1.attachInterrupt( ScanDMD );   //attach the Timer1 interrupt to ScanDMD which goes to dmd.scanDisplayBySPI()

   //clear/init the DMD pixels held in RAM
   dmd.clearScreen( true );   //true is normal (all pixels off), false is negative (all pixels on)
   //clear LED matrix - off all LED
   // 5 x 7 font timer, including demo of OR and NOR modes for pixels so that the flashing colon can be overlayed
   //dmd.clearScreen( true );
   dmd.selectFont(SystemFont5x7);
   // initial value equally "00:00:00"
   dmd.drawChar(  0,  0, '0', GRAPHICS_NORMAL );
   dmd.drawChar(  5,  0, '0', GRAPHICS_NORMAL );
   dmd.drawChar( 13,  0, '0', GRAPHICS_NORMAL );
   dmd.drawChar( 18,  0, '0', GRAPHICS_NORMAL );
   dmd.drawChar( 24,  0, '0', GRAPHICS_NORMAL );
   dmd.writePixel( 11,  2, GRAPHICS_NORMAL, 1 );
   dmd.writePixel( 11,  4, GRAPHICS_NORMAL, 1 );
   dmd.writePixel( 23,  6, GRAPHICS_NORMAL, 1 ); 

    //define port to look dada from distance sensor
    Serial.begin(9600);
    Wire.begin();

    //Initialization sensor VL53L0X
    sensor.setTimeout(500);
    if (!sensor.init())
    {
      while (1){ 
         // out put error message when the sensor is not initialized 
         Serial.println("Failed to detect and initialize sensor!");
         dmd.drawMarquee("Failed to detect and initialize sensor!",39,(32*DISPLAYS_ACROSS)-1,5);
         long start=millis();
         long timer=start;
         boolean ret=false;
         while(!ret){
            if ((timer+50) < millis()) {
              ret=dmd.stepMarquee(-1,0);
              timer=millis();
            }
         }
      }  
     }

    // reduce timing budget to 20 ms (default is about 33 ms). For HIGH_SPEED mode of distance sensor. 
  //  sensor.setMeasurementTimingBudget(20000);
   // increase timing budget to 20 ms (default is about 33 ms). For HIGH_ACCURACY mode of distance sensor. 
  //  sensor.setMeasurementTimingBudget(200000);
  // понижает предел скорости обратного сигнала (по умолчанию 0,25 MCPS (мчип/с))
  sensor.setSignalRateLimit(0.1);
  // увеличить периоды лазерного импульса (по умолчанию 14 и 10 PCLK)
  // * - PCLK — это частота периферии
  sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
  sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
}


void loop() 
{
  
   for (;;)
   {
      int distanceLim  = 1250;
      lap++;
      flagDraw = false;
      String strLap = String(lap, DEC);
      strLap.toCharArray(charLap, 5);
      dmd.drawString(0,8, "No",3,GRAPHICS_NORMAL);
      dmd.drawString(13,8, charLap,5,GRAPHICS_NORMAL);
      for( int minuteDecade = 0; minuteDecade <= 5; minuteDecade++)
      {
      
          for( int minuteUnit = 0; minuteUnit <= 9; minuteUnit++) 
          {
            
              for( int secondDecade = 0; secondDecade <= 5; secondDecade++)
              {
                  
                  for( int secondUnit = 0; secondUnit <= 9; secondUnit++)
                 {
                         
                       for( int i = 0; i <= 9; i++) //tenths of second
                       {
                            timeZero = millis(); 
                            distanceZero = sensor.readRangeSingleMillimeters();
                            Serial.println(distanceZero);
                          
                            if (lap == 1){
                                dmd.drawChar( 0,  0, minuteDecade+'0', GRAPHICS_NORMAL );
                                dmd.drawChar( 5,  0, minuteUnit+'0', GRAPHICS_NORMAL );
                                dmd.drawChar( 13,  0, secondDecade+'0', GRAPHICS_NORMAL );
                                dmd.drawChar( 18,  0, secondUnit+'0', GRAPHICS_NORMAL );
                                dmd.drawChar( 24,  0, i+'0', GRAPHICS_NORMAL );//draw new value of hundredths of second 
                                                        
                               } else if ((secondDecade > 0) && (flagDraw == false)){
                                flagDraw = true;
                              
                               } else if (flagDraw == true){
                                dmd.drawChar( 0,  0, minuteDecade+'0', GRAPHICS_NORMAL );
                                dmd.drawChar( 5,  0, minuteUnit+'0', GRAPHICS_NORMAL );
                                dmd.drawChar( 13,  0, secondDecade+'0', GRAPHICS_NORMAL );
                                dmd.drawChar( 18,  0, secondUnit+'0', GRAPHICS_NORMAL );
                                dmd.drawChar( 24,  0, i+'0', GRAPHICS_NORMAL );//draw new value of hundredths of second 
                               }
                              distance = sensor.readRangeSingleMillimeters();    
                              Serial.println(distance);
                            _time = millis();
                         //   Serial.println(_time);
                             delay(100-_time+timeZero);    
                         //  Serial.println(_time-timeZero);
                         //  Serial.println(abs(distance-distanceZero));// in this case Serial.println operator necessary for debugging
                   
                        if ((distanceZero < distanceLim) && (distance < distanceLim)) break;
                        } 
                   if ((distanceZero < distanceLim) && (distance < distanceLim)) break;
                  }
              if ((distanceZero < distanceLim) && (distance < distanceLim)) break;   
              }
          if ((distanceZero < distanceLim) && (distance < distanceLim)) break;    
          }
      if ((distanceZero < distanceLim) && (distance < distanceLim)) break;   
      }
      
   }
   
}
