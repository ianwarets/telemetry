#include <printf.h>
#include <VirtualWire.h>

#define SENSORPIN 2
#define LASERINDICATOR 6
#define TX_PIN 8
unsigned long timerDelay = 1500;
void isrSaveTime();
volatile unsigned long irqTime = 0;
volatile bool interrupt = false;
unsigned int msgCounter = 0;
const int sensorCode = 731;
const unsigned long initialized = 700002;
const unsigned long ready = 700001;
const unsigned long noSignal = 700000;
bool sensorIrqEnabled = false;
const unsigned long heartBeatInterval = 1000;
unsigned long lastRadioSent = 0;

struct message {
    int code;
    unsigned int msgCounter;
    unsigned long time;
};

void setup(){
    vw_set_tx_pin(TX_PIN);
    vw_setup(1200);    
    pinMode(SENSORPIN, INPUT);
    // Сообщение, обозначающее начало работы датчика и наличие подключения датчика.
    message msg {
        sensorCode,
        msgCounter,
        initialized
    };
    radioSendMessage(msg);
    enableSensorIrq();
}

void loop(){
    static unsigned long result = 0;
    static unsigned long prevResult = 0;
    static unsigned long lastNoSignal = 0;
    // No signal from sensor. Signal = 0. No signal = 1;
    int signal = !digitalRead(SENSORPIN);
    digitalWrite(LASERINDICATOR, signal);
    if(!signal && lastNoSignal == 0){
        lastNoSignal = millis();
    }
    if(signal){
        lastNoSignal = 0;
    }
    if((!signal&& lastNoSignal + timerDelay < millis())){
// При отсутствии сигнала в течение 1.5 секунд включаем возвращаем noSignal
        disableSensorIrq();
        sensorIrqEnabled = false;
        delay(500);
        result = noSignal;
    }else{
        if(!sensorIrqEnabled){
            delay(1000);
            enableSensorIrq();
            sensorIrqEnabled = true;
            result = ready;
        }else{
            if(irqTime == 0){
                result = ready;
            }else if(interrupt){
                result = irqTime;
                interrupt = false;
            }
        }
    }
    if(result != prevResult){
        msgCounter++;
        prevResult = result;
        sendResultToDisplay(result);
    }else if(lastRadioSent + heartBeatInterval < millis()){
        sendResultToDisplay(result);
    }
}
void sendResultToDisplay(unsigned long result){
    message msg {
            sensorCode,
            msgCounter,
            result
        };
    radioSendMessage(msg);
}
void radioSendMessage(message msg){
    for(unsigned short i = 0; i < 3; i++){
        vw_send((uint8_t *)&msg, sizeof(msg));
        vw_wait_tx();
    }
    lastRadioSent = millis();
}
void isrSaveTime(){
    unsigned long now = millis();
    if(irqTime + timerDelay < now){
        irqTime = now;
        interrupt = true;
    }
}

void enableSensorIrq(){
    attachInterrupt(digitalPinToInterrupt(SENSORPIN), isrSaveTime, RISING);
    sensorIrqEnabled = true;
}

void disableSensorIrq(){
    detachInterrupt(digitalPinToInterrupt(SENSORPIN));
    sensorIrqEnabled = false;
}