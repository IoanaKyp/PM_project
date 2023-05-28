#include <LiquidCrystal_I2C.h> // I2C LCD
//---------------------------------------------------------------------
#include <FastLED.h>

#define LED_PIN 3
#define NUM_LEDS 12
CRGB leds[NUM_LEDS];
CRGB colors[] = {CRGB(255, 0, 255), CRGB(255, 100, 0), CRGB(0, 255, 255), CRGB(255, 255, 0),CRGB(2047, 153, 255), CRGB(102, 255, 255)};
volatile int current_color = 0;
const int buttonPin = 9;
int buttonState = 0; 
int buttonLast = 1; 
volatile int adding = 10;
//---------------------------------------------------------------------

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

 
// initial Time display is 12:59:45 PM
int h=12;
int m=59;
int s=45;
int flag=1; //PM

// Time Set Buttons
int button1;
int button2;

// Pins definition for Time Set Buttons
int hs=0;// pin 0 for Hours Setting
int ms=1;// pin 1 for Minutes Setting

// Backlight Time Out 
const int Time_light=150;
int bl_TO=Time_light;// Backlight Time-Out


// For accurate Time reading, use Arduino Real Time Clock and not just delay()
static uint32_t last_time, now = 0; // RTC


void setup()
{
  lcd.init();
  lcd.backlight();
  lcd.clear();
    
  pinMode(hs,INPUT_PULLUP);// avoid external Pullup resistors for Button 1
  pinMode(ms,INPUT_PULLUP);// and Button 2
  now=millis(); // read RTC initial value  
  
  pinMode(13,OUTPUT); // shut down Led
  digitalWrite(13,LOW);

//----------------------------------------------------------------------
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  FastLED.setBrightness(20);
  FastLED.clear();
  FastLED.show();

  pinMode(buttonPin, INPUT_PULLUP);
//----------------------------------------------------------------------
}


void loop()
{ 
 lcd.clear(); // every second
// Update LCD Display
// Print TIME in Hour, Min, Sec + AM/PM
 lcd.setCursor(0,0);
 lcd.print("Time ");
 if(h<10)lcd.print("0");// always 2 digits
 lcd.print(h);
 lcd.print(":");
 if(m<10)lcd.print("0");
 lcd.print(m);
 lcd.print(":");
 if(s<10)lcd.print("0");
 lcd.print(s);

 if(flag==0) lcd.print(" AM");
 if(flag==1) lcd.print(" PM");
 
 lcd.setCursor(0,1);// for Line 2
 lcd.print("      ^_^       ");

  // improved replacement of delay(1000) 
  // Much better accuracy, no more dependant of loop execution time

  for ( int i=0 ;i<5 ;i++)// make 5 time 200ms loop, for faster Button response
  {
    while ((now-last_time)<200) //delay200ms
    { 
      now=millis();
    }
    // inner 200ms loop
    last_time=now; // prepare for next loop 
   //--------------------------------------------------------------------------
   buttonState = digitalRead(buttonPin);
  //   if (buttonState && !buttonLast)
  //   {
  //       // Button event here
  //       current_color = (current_color + 1) % 6;
  //       delay(100);  // <---- Delay inserted after the button event
  //   }

  //   //Update button flag
  //   buttonLast = buttonState;

    if(buttonState == 1) current_color = (current_color + 1) % 6;

    fill_solid(leds, NUM_LEDS, colors[current_color]);
    FastLED.show();
   //--------------------------------------------------------------------------

    // read Setting Buttons
    button1=digitalRead(hs);// Read Buttons
    button2=digitalRead(ms);

    //Backlight time out 
    bl_TO--;
    if(bl_TO==0)
    {
      lcd.noBacklight();
      bl_TO++;
    }
    
    // Hit any to activate Backlight 
    if(  ((button1==0)|(button2==0)) & (bl_TO==1) ) {
      bl_TO=Time_light;
      lcd.backlight();
      // wait until Button released
      while ((button1==0)|(button2==0))
      {
      button1=digitalRead(hs);// Read Buttons
      button2=digitalRead(ms);
      }
    } else {
    // Process Button 1 or Button 2 when hit while Backlight on 
      if(button1==0){
        h=h+1;
        bl_TO=Time_light;
        lcd.backlight();
      }

      if(button2==0){
        s=0; m=m+1;
        bl_TO=Time_light;
        lcd.backlight();
        }

      /* ---- manage seconds, minutes, hours am/pm overflow ----*/
      if(s==60){ s=0; m=m+1;}
      if(m==60){ m=0; h=h+1;}
      if(h==13){ h=1; flag=flag+1; if(flag==2) flag=0;}

      if((button1==0)|(button2==0)) {// Update display if time set button pressed
        // Update LCD Display
        // Print TIME in Hour, Min, Sec + AM/PM
        lcd.setCursor(0,0);
        lcd.print("Time ");
        if(h<10)lcd.print("0");// always 2 digits
        lcd.print(h);
        lcd.print(":");
        if(m<10)lcd.print("0");
        lcd.print(m);
        lcd.print(":");
        if(s<10)lcd.print("0");
        lcd.print(s);

        if(flag==0) lcd.print(" AM");
        if(flag==1) lcd.print(" PM");
      
        lcd.setCursor(0,1);// for Line 2
        lcd.print("      ^_^       ");
      }
    } // end if else
  }// end for

  // outer 1000ms loop
  s=s+1; //increment sec. counting
      
  // ---- manage seconds, minutes, hours am/pm overflow ----
  if(s==60){ s=0; m=m+1;}
  if(m==60){ m=0; h=h+1;}
  if(h==13){ h=1; flag=flag+1; if(flag==2) flag=0;} 
}