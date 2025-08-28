#include <printf.h>
#include <VirtualWire.h>

#define SENSORPIN 2
#define TX_PIN 8
unsigned long timerDelay = 1500;
void isrSaveTime();
volatile unsigned long timerTime = 0, prevTime = 0;
volatile bool interrupt = false;

void setup(){
    vw_set_tx_pin(TX_PIN);
    vw_setup(1200);    
    pinMode(SENSORPIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(SENSORPIN), isrSaveTime, RISING);
    char * msg = "SEM71111";
    vw_send((uint8_t *)msg, strlen(msg));
    vw_wait_tx();
    delay(1000);
    msg = "SEM000000";
    vw_send((uint8_t *)msg, strlen(msg));
    vw_wait_tx();
}

void loop(){
    if(interrupt){
        unsigned long result = timerTime - prevTime;
        char msg[30];
        sprintf(msg, "SEM%lu", result);
        vw_send((uint8_t *)msg, sizeof(msg));
        vw_wait_tx();
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