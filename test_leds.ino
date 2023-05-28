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

int button_press(int buttonPin) {
  for (int i = 0; i < 10; i++) {
    buttonState = digitalRead(buttonPin);
    if(buttonState == HIGH)
      return 0;
    delay(50);
  }
  return 1;
}

void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  FastLED.setBrightness(20);
  FastLED.clear();
  FastLED.show();


  pinMode(buttonPin, INPUT_PULLUP);

  Serial.begin(9600);


}
 
void loop() {
  // RED Green Blue
  buttonState = digitalRead(buttonPin);

  if (buttonState && !buttonLast)
    {
        // Button event here
        current_color = (current_color + 1) % 6;
        adding = (adding + 10)%200;
        //delay(10);
        Serial.println(adding);
        // Serial.println(colors[current_color]);
        delay(100);  // <---- Delay inserted after the button event
    }

    //Update button flag
    buttonLast = buttonState;


  fill_solid(leds, NUM_LEDS, colors[current_color]);
  FastLED.show();
}