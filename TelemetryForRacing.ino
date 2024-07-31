#include <nRF24L01.h>
#include <printf.h>
#include <RF24_config.h>
#include <RF24.h>

#define SENSORPIN 3
#define RF24CEPIN 4
#define RF24CSNPIN 5
unsigned long timerDelay = 1500;
void isrSaveTime();
volatile unsigned long timerTime = 0, prevTime = 0;
unsigned int minutes, seconds, milliseconds;
volatile bool interrupt = false;
RF24 radio(RF24CEPIN, RF24CSNPIN);

void setup(){
    radio.begin();
    radio.setChannel(5);
    radio.setDataRate(RF24_250KBPS);
    radio.setPALevel(RF24_PA_LOW);
    /*
        Уникальный идентификатор канала передачи
    */
    radio.openWritingPipe(0x7878787878LL);

    pinMode(SENSORPIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(SENSORPIN), isrSaveTime, FALLING);

}

void loop(){
    if(interrupt){
        unsigned long result = timerTime - prevTime;
        radio.write(&result, sizeof(result));
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