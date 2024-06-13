#include <Arduino.h>
#line 1 "C:\\Users\\ianwa\\OneDrive\\Документы\\Arduino\\Sketches\\TelemetryForRacing\\TelemetryForRacing.ino"
#include <EEPROM.h>
#include <Wire.h>
//#include <LiquidCrystal_I2C.h>
#include <LCDI2C_Multilingual.h>

uint8_t sensorPin = 2;
void isrSaveTime();
void saveRecordToMemory(unsigned int);
volatile unsigned long timerTime = 0, prevInterruptTime = 0;
volatile bool isRunning = false;
unsigned int minutes, seconds, milliseconds;
volatile unsigned int interruptCounter = 0;

LCDI2C_Russian  lcd(0x27, 16, 2);

#line 16 "C:\\Users\\ianwa\\OneDrive\\Документы\\Arduino\\Sketches\\TelemetryForRacing\\TelemetryForRacing.ino"
void setup();
#line 25 "C:\\Users\\ianwa\\OneDrive\\Документы\\Arduino\\Sketches\\TelemetryForRacing\\TelemetryForRacing.ino"
void loop();
#line 57 "C:\\Users\\ianwa\\OneDrive\\Документы\\Arduino\\Sketches\\TelemetryForRacing\\TelemetryForRacing.ino"
void saveRecordToMemory(unsigned int timeToSave);
#line 61 "C:\\Users\\ianwa\\OneDrive\\Документы\\Arduino\\Sketches\\TelemetryForRacing\\TelemetryForRacing.ino"
void printTime(unsigned int time);
#line 16 "C:\\Users\\ianwa\\OneDrive\\Документы\\Arduino\\Sketches\\TelemetryForRacing\\TelemetryForRacing.ino"
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
    lcd.print(interruptCounter);
    if(isRunning){
        unsigned long currentTime = millis() - timerTime;
        lcd.print("Timer started!");
        printTime(currentTime);
    }else{
        static int prevInterruptCounter;
        if(prevInterruptCounter < interruptCounter){
            saveRecordToMemory(timerTime);
        }
        lcd.print("Timer stopped!");
        printTime(timerTime);
    }
}

void isrSaveTime(){
    interruptCounter++;
    unsigned int now = millis();

    if(prevInterruptTime == 0 || prevInterruptTime + 3000 < now){
        if(isRunning){
            timerTime = now - timerTime;
        }else{
            timerTime = now;
        }
        isRunning = !isRunning;
    }
    prevInterruptTime = now;
}

void saveRecordToMemory(unsigned int timeToSave){
    return;
}

void printTime(unsigned int time){
    char timeString[10]; 
    unsigned short minutes = (time/1000)/60;
    unsigned short seconds = (time/1000)%60;
    unsigned short milliseconds = time%1000;
    sprintf(timeString, "%02d:%02d:%03d\0", minutes, seconds, milliseconds);
    lcd.setCursor(0, 1);
    lcd.print("time:");
    lcd.print(timeString);
}
