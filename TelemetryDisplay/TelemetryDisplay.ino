#include <nRF24L01.h>
#include <printf.h>
#include <RF24_config.h>
#include <RF24.h>

#define RF24CEPIN 4
#define RF24CSNPIN 5

#define DATA_PIN 6
#define CLOCK_PIN 7
#define LATCH_PIN 8
#define DIGITS_COUNT 10
#define DISPLAY_SIZE 6

unsigned long timerDelay = 1500;
volatile unsigned long timerTime = 0, prevTime = 0;
RF24 radio(RF24CEPIN, RF24CSNPIN);

byte digitsDot[DIGITS_COUNT] = {
    0b11111101,
    0b01100001,
    0b11011011,
    0b11110011,
    0b01100111,
    0b10110111,
    0b10111111,
    0b11100001,
    0b11111111,
    0b11110111
};
byte digitsNoDot[DIGITS_COUNT] = {
    0b11110110,
    0b01100000,
    0b11010101,
    0b11110001,
    0b01100011,
    0b10110011,
    0b10110111,
    0b11100000,
    0b11110111,
    0b11110011
};

void setup(){
    pinMode(DATA_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(LATCH_PIN, OUTPUT);
    timeToDisplay(0);
    delay(1000);
    radio.begin();
    radio.setChannel(5);
    radio.setDataRate(RF24_250KBPS);
    radio.setPALevel(RF24_PA_LOW);
    /*
        Уникальный идентификатор канала передачи
    */
    radio.openReadingPipe(1, 0x7878787878LL);
    radio.startListening();
}

void loop(){
    unsigned long time[1];
    if(radio.available()){
        radio.read(time, sizeof(time));
        timeToDisplay(time[0]);
    }
}

void timeToDisplay(unsigned long time){
    int secondsAll = time/1000;
    int minutes = (secondsAll/60)%10;
    int seconds = secondsAll%60;
    int milliseconds = time%1000;
    int sec1Digit = seconds/10;
    int sec2dDigit = seconds%10;
    int ms1Digit = milliseconds/100;
    int ms2Digit = (milliseconds%100)/10;
    int ms3Digit = milliseconds%10;
    byte record[DISPLAY_SIZE] = {digitsNoDot[ms1Digit], digitsNoDot[ms2Digit], digitsNoDot[ms3Digit], digitsDot[minutes], digitsNoDot[sec1Digit], digitsDot[sec2dDigit]};
    writeRecord(record);
}
void writeRecord(byte b[DISPLAY_SIZE]){
    PBdigWL(LATCH_PIN);
    for(unsigned short i = 0; i < DISPLAY_SIZE; i++){
        out_595_port(b[i]);
    }
    PBdigWH(LATCH_PIN);
}
void out_595_port(byte x){
    byte ii = 0b00000001;
    for(int i = 0; i <=7; i++){
        if(ii & x){
            PBdigWH(DATA_PIN);
        }else{
            PBdigWL(DATA_PIN);
        }
        ii <<= 1;
        PBdigWH(CLOCK_PIN);
        PBdigWL(CLOCK_PIN);
    }
}

inline void PBdigWH(byte b){
    digitalWrite(b, 1);
}

inline void PBdigWL(byte b){
    digitalWrite(b, 0);
}