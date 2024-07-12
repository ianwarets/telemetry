#include <Wire.h>
#include <LiquidCrystal_I2C.h>
//#include <LCDI2C_Multilingual.h>

uint8_t sensorPin = 3;
unsigned long timerDelay = 1500;
void isrSaveTime();
void saveRecordToMemory(unsigned int);
void printTime(unsigned long, short, short);
volatile unsigned long timerTime = 0, prevTime = 0;
unsigned int minutes, seconds, milliseconds;
volatile bool interrupt = false;
#ifdef DEBUG
volatile unsigned int interruptCounter = 0;
#endif

LiquidCrystal_I2C  lcd(0x27, 16, 2);

void setup(){
    lcd.init();
    lcd.backlight();
    lcd.clear();
    // Init I/O pins
    pinMode(sensorPin, INPUT);
    attachInterrupt(digitalPinToInterrupt(sensorPin), isrSaveTime, FALLING);
}

void loop(){
    lcd.setCursor(0, 0);
    #ifdef DEBUG
    lcd.print(interruptCounter);
    #endif

    lcd.print("Now: ");
    printTime(millis() - timerTime, 6, 0);
    if(interrupt){
        lcd.setCursor(0, 1);
        lcd.print("Prev: ");
        printTime(timerTime - prevTime, 6, 1);
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
    #ifdef DEBUG
    interruptCounter++;
    #endif
}

void saveRecordToMemory(unsigned int timeToSave){
}

void printTime(unsigned long time, short col, short row){
    char timeString[10]; 
    unsigned int secondsAll = time/1000;
    unsigned int minutes = secondsAll/60;
    unsigned int seconds = secondsAll%60;
    unsigned int milliseconds = time%1000;
    sprintf(timeString, "%02d:%02d:%03d\0", minutes, seconds, milliseconds);
    lcd.setCursor(col, row);
    lcd.print(timeString);
}