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
#ifdef DEBUG
volatile unsigned int interruptCounter = 0;
#endif

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
    static bool resultPrinted = true;
    if(isRunning){
        unsigned long currentTime = millis() - timerTime;
        lcd.print("Timer started!");
        printTime(currentTime);
        resultPrinted = false;
    }else{
        if(!resultPrinted){
            saveRecordToMemory(timerTime);
            delay(1000);
            lcd.clear();
            resultPrinted = true;
        }else{
            if(prevInterruptTime + 3000 > millis()){
                lcd.print("Wait 3 sec.");
            }else{
                lcd.print("Get ready! ");
            }
        }
        printTime(timerTime);
    }
}

void isrSaveTime(){
    unsigned long now = millis();
    if(prevInterruptTime + 3000 < now){
        if(isRunning){
            timerTime = now - timerTime;
        }else{
            timerTime = now;
        }
        isRunning = !isRunning;
        prevInterruptTime = now;
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