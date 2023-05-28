#include <LiquidCrystal_I2C.h>
#include <FastLED.h>

#define NUM_LEDS 12       //number of leds in the strip
#define LED_PIN 3         //pin for signal to control led strip 
#define BUTTON_LED_CTRL 9 //pin for button used in switching led colors
#define BUTTON_H_CTRL 0   //pin for button used to set hours
#define BUTTON_M_CTRL 1   //pin for button used to set minutes
#define LIGHT_TIME 150    //time for screen light

int buttonState_ledCtrl = 0; 
int buttonLast_ledCtrl = 1; 

CRGB leds[NUM_LEDS];
CRGB colors[] = {CRGB(255, 0, 255),   // magenta
                 CRGB(255, 100, 0),   // orange
                 CRGB(0, 255, 255),   // blue
                 CRGB(255, 255, 0),   // yellow
                 CRGB(204, 153, 255), // lila
                 CRGB(102, 255, 255)  // light blue
                 };

volatile int current_color = 0; // Index for color in the colors array


// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2); 

 
// Initial Time display is 9:59:00 AM
int h=9;
int m=59;
int s=0;
int flag=0; //AM

// Variables used for reading the inputs from control buttons
int hour_signal;
int min_signal;

// Backlight Time Out 
int bl_TO=LIGHT_TIME;// Backlight time-out

// For accurate time reading, use Arduino Real Time Clock and not just delay()
static uint32_t last_time, now = 0; // Initialise counters for time intervals

void setup()
{
  //////////////////////////////// LCD setup //////////////////////////////////////////
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,1);// For Line 2
  lcd.print("      ^_^       ");
    
  //////////////////////////////// PINS setup //////////////////////////////////////////  
  pinMode(BUTTON_H_CTRL,INPUT_PULLUP);
  pinMode(BUTTON_M_CTRL,INPUT_PULLUP);
  pinMode(BUTTON_LED_CTRL, INPUT_PULLUP);
  
  pinMode(13,OUTPUT); // Shut down Led - optional
  digitalWrite(13,LOW);

  now=millis(); // Read RTC initial value  
  //////////////////////////////// LED STRIP setup //////////////////////////////////////
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS); // Declare leds used
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);        // Safety
  FastLED.setBrightness(20); 
  FastLED.clear();   
  FastLED.show();

  //////////////////////////////// LEV PINS setup //////////////////////////////////////
  DDRC |= (1 << PC0) | (1 << PC1); // A0 and A1 will give the control signals
  // Initialize Timer1
  PORTC |= (1 << PC0);   // Initialise A0 with value 1
  PORTC &= ~(1 << PC1) ; // Initialise A1 with value 0
  noInterrupts(); // Disable interrupts

  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 200; // Set compare register (16MHz / 200 = 80kHz square wave -> 40kHz full wave)
  TCCR1B |= (1 << WGM12); // CTC mode
  TCCR1B |= (1 << CS10); // Set prescaler to 1 ==> no prescaling
  TIMSK1 |= (1 << OCIE1A); // Enable compare timer interrupt

  interrupts(); // Enable interrupts
}

// ISR for the transmitters
ISR(TIMER1_COMPA_vect) {
  // Toggle the outputs used to control the transmitters
  PORTC ^= (1 << PC0) ^ (1 << PC1); 
}

void loop() { 
  if (s == 0) { // Update LCD Display every minute
    // Print TIME in format Hour:Min + AM/PM
    lcd.setCursor(0,0);
    lcd.print("Time   ");
    if (h < 10) // always 2 digits
      lcd.print("0"); 
    lcd.print(h);
    lcd.print(":");
    if (m < 10) // always 2 digits
      lcd.print("0"); 
    lcd.print(m);

    if (flag == 0) lcd.print("  AM");
    if (flag == 1) lcd.print("  PM");
  }

  for (int i = 0; i < 5; i++)// make 5 200ms loops, for faster Button response
  {
    while ((now - last_time) < 200) // Delay 200ms
      now = millis();
    last_time=now; // Prepare for next loop 

    //------------------------------ led control ---------------------------------
    buttonState_ledCtrl = digitalRead(BUTTON_LED_CTRL);
    //if button pressed switch to next color
    if(buttonState_ledCtrl == 1) {
      current_color = (current_color + 1) % 6;
    }

    for(int i = 0; i < NUM_LEDS; i++) {
      if (i == NUM_LEDS - 2) 
        leds[i] = colors[(current_color + 1)%6];
      else if (i == NUM_LEDS - 1) 
        leds[i] = colors[(current_color + 2)%6];
      else 
        leds[i] = colors[current_color];
    }
    FastLED.show();

   //-------------------------- clock logic --------------------------------------------

    // Read setting Buttons
    hour_signal = digitalRead(BUTTON_H_CTRL);
    min_signal = digitalRead(BUTTON_M_CTRL);

    //Backlight time out 
    bl_TO--;
    if (bl_TO == 0) {
      lcd.noBacklight();
      bl_TO++;
    }
    
    // Hit any butotn to activate Backlight if light off
    if (((hour_signal == 0) | (min_signal == 0)) & (bl_TO == 1)) {
      bl_TO=LIGHT_TIME;
      lcd.backlight();
      // Wait until button released
      while ((hour_signal == 0) | (min_signal == 0)) {
        hour_signal = digitalRead(BUTTON_H_CTRL);
        min_signal = digitalRead(BUTTON_M_CTRL);
      }
    } else {
      // Process setting the hour/minute when buttons are hit while Backlight on 
      if (hour_signal == 0){
        h = h + 1;
        bl_TO = LIGHT_TIME;
        lcd.backlight();
      }

      if (min_signal == 0){
        s = 0; 
        m = m + 1;
        bl_TO = LIGHT_TIME;
        lcd.backlight();
      }

      // Manage overflow
      if (s == 60) { 
        s = 0; 
        m = m + 1;
      }
      if (m == 60) { 
        m = 0; 
        h = h + 1;
      }
      if (h == 13) { 
        h = 1; 
        flag = flag + 1;
        if (flag == 2) 
          flag=0;
      }

      if((hour_signal==0)|(min_signal==0)) {// Update display if time set button pressed
        lcd.setCursor(0,0);
        lcd.print("Time   ");
        if (h < 10) // always 2 digits
          lcd.print("0");
        lcd.print(h);
        lcd.print(":");
        if (m < 10) // always 2 digits
          lcd.print("0");
        lcd.print(m);

        if (flag == 0) lcd.print("  AM");
        if (flag == 1) lcd.print("  PM");
      }
    } 
  }

  s = s + 1; //increment sec. at every loop (5x200 = 1000ms = 1s)
      
  // Manage seconds overflow
  if (s == 60) { 
    s = 0; 
    m = m + 1;
  }
  if (m == 60) { 
    m = 0; 
    h = h + 1;
  }
  if (h == 13) { 
    h = 1; 
    flag = flag + 1; 
    if (flag == 2) 
    flag=0;
  } 
}