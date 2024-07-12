#include <Wire.h>
#include <LiquidCrystal_I2C.h>
//#include <LCDI2C_Multilingual.h>

uint8_t sensorPin = 3;
unsigned long timerDelay = 1500;
void isrSaveTime();
void saveRecordToMemory(unsigned int);
volatile unsigned long timerTime = 0, prevInterruptTime = 0;
volatile bool isRunning = false;
unsigned int minutes, seconds, milliseconds;
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
    static bool resultPrinted = true;
    if(isRunning){
        lcd.print("Timer started!");
        printTime(millis() - timerTime);
        resultPrinted = false;
    }else{
        if(!resultPrinted){
            lcd.clear();
            saveRecordToMemory(timerTime);
            delay(1000);
            lcd.clear();
            resultPrinted = true;
        }

        if(prevInterruptTime + timerDelay > millis()){
            lcd.print("Wait 3 sec.");
        }else{
            lcd.print("Get ready! ");
        }
        printTime(timerTime);
    }
}

void isrSaveTime(){
    unsigned long now = millis();
    if(prevInterruptTime + timerDelay < now){
        if(isRunning){
            timerTime = now - timerTime;
        }else{
            timerTime = now;
        }
        prevInterruptTime = now;
        isRunning = !isRunning;
    }
    #ifdef DEBUG
    interruptCounter++;
    #endif
}

void saveRecordToMemory(unsigned int timeToSave){
    lcd.print("Time recorded.");
}

void printTime(unsigned long time){
    char timeString[10]; 
    unsigned int secondsAll = time/1000;
    unsigned int minutes = secondsAll/60;
    unsigned int seconds = secondsAll%60;
    unsigned int milliseconds = time%1000;
    sprintf(timeString, "%02d:%02d:%03d\0", minutes, seconds, milliseconds);
    lcd.setCursor(4, 1);
    lcd.print(timeString);
}