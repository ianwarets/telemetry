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
    if(isRunning){
        unsigned long currentTime = millis() - timerTime;
        lcd.print("Timer started!");
        printTime(currentTime);
    }else{
        static int prevInterruptCounter;
        if(prevInterruptCounter < interruptCounter){
            saveRecordToMemory(timerTime);
        }
        if(prevInterruptTime + 3000 < millis()){
            lcd.setCursor(0, 0);
            lcd.print("Wait 3 sec.");
        }else{
            lcd.print("Get ready!");
        }
        printTime(timerTime);
    }
}

void isrSaveTime(){
    interruptCounter++;
    unsigned int now = millis();

    if(prevInterruptTime + 3000 < now || prevInterruptTime == 0){
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
    lcd.setCursor(0,0);
    lcd.print("Time recorded.");
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