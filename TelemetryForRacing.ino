#include <FastLED.h>

#define LED_PIN  5
 
#define COLOR_ORDER RGB
#define CHIPSET     WS2812B
 
#define BRIGHTNESS 255

#define SENSORPIN 3
unsigned long timerDelay = 1500;
void isrSaveTime();
void printTime(unsigned long);
volatile unsigned long timerTime = 0, prevTime = 0;
volatile bool interrupt = false;

const uint8_t font5x7[][5] PROGMEM = {
  {0x3e, 0x41, 0x41, 0x41, 0x3e}, // 0 0x30 48
  {0x00, 0x42, 0x7f, 0x40, 0x00}, // 1 0x31 49
  {0x42, 0x61, 0x51, 0x49, 0x46}, // 2 0x32 50
  {0x21, 0x41, 0x45, 0x4b, 0x31}, // 3 0x33 51
  {0x18, 0x14, 0x12, 0x7f, 0x10}, // 4 0x34 52
  {0x27, 0x45, 0x45, 0x45, 0x39}, // 5 0x35 53
  {0x3c, 0x4a, 0x49, 0x49, 0x30}, // 6 0x36 54
  {0x01, 0x71, 0x09, 0x05, 0x03}, // 7 0x37 55
  {0x36, 0x49, 0x49, 0x49, 0x36}, // 8 0x38 56
  {0x06, 0x49, 0x49, 0x29, 0x1e}, // 9 0x39 57
};

// Params for width and height
#define WIDTH 32
#define HEIGHT 8

#define NUM_LEDS (WIDTH * HEIGHT)
CRGB leds_plus_safety_pixel[ NUM_LEDS + 1];
CRGB* const leds( leds_plus_safety_pixel + 1);
 
// Param for different pixel layouts
const bool kMatrixSerpentineLayout = true;
const bool kMatrixVertical = true;



void setup(){
    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
    FastLED.setBrightness( BRIGHTNESS );
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 1000);
    // Init I/O pins
    pinMode(SENSORPIN, INPUT);
    
    attachInterrupt(digitalPinToInterrupt(SENSORPIN), isrSaveTime, CHANGE);
    printTime(0);
}

void loop(){
    if(interrupt){
        printTime(timerTime - prevTime);
        interrupt = false;
    }
}

void isrSaveTime(){
    unsigned long now = millis();
    if(timerTime + timerDelay < now){
        prevTime = timerTime;
        timerTime = now;
        interrupt = true;
    }
}

void printTime(unsigned long time){
    FastLED.clear();
    int secondsAll = time/1000;
    int minutes = (secondsAll/60)%10;
    int seconds = secondsAll%60;
    int milliseconds = time%1000;
    int sec1Digit = seconds/10;
    int sec2Digit = seconds%10;
    int ms1Digit = milliseconds/100;
    int ms2Digit = (milliseconds%100)/10;
    int ms3Digit = milliseconds%10;
    drawDigit5x7(minutes, 0, 0, CRGB::Green);
    drawPixelXY(5, 0, CRGB::Cyan); 
    drawDigit5x7(sec1Digit, 6, 1, CRGB::Red);
    drawDigit5x7(sec2Digit, 11, 0, CRGB::Green);
    drawPixelXY(16, 0, CRGB::Cyan);
    drawDigit5x7(ms1Digit, 17, 1, CRGB::Red);
    drawDigit5x7(ms2Digit, 22, 0, CRGB::Green);
    drawDigit5x7(ms3Digit, 27, 1, CRGB::Red);
    FastLED.show();
}

void drawDigit5x7(byte digit, byte X, byte Y, CRGB color) {
  if (digit > 9) return;
  for (byte x = 0; x < 5; x++) {
    byte thisByte = pgm_read_byte(&(font5x7[digit][x]));
    for (byte y = 0; y < 7; y++) {
      if (x + X > WIDTH || y + Y > HEIGHT) continue;
      if (thisByte & (1 << 6 - y)) drawPixelXY(x + X, y + Y, color);
    }
  }
}

void drawPixelXY(int8_t x, int8_t y, CRGB color) {
  if (x < 0 || x > WIDTH - 1 || y < 0 || y > HEIGHT - 1) return;
  int thisPixel = getPixelNumber(x, y);
    leds[thisPixel] = color;
}

uint16_t getPixelNumber(int8_t x, int8_t y) {
  if (((WIDTH - x - 1) % 2 == 0)) {               // если чётная строка
    return ((WIDTH - x - 1) * HEIGHT + y);
  } else {                                              // если нечётная строка
    return ((WIDTH - x - 1) * HEIGHT + HEIGHT - y - 1);
  }
}