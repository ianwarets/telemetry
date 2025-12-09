#include <VirtualWire.h>

#define DATA_PIN 6
#define CLOCK_PIN 7
#define LATCH_PIN 8
#define DIGITS_COUNT 10
#define DISPLAY_SIZE 6
#define RX_PIN 3
#define SENSORPIN 2
unsigned long timerDelay = 1500;
void isrSaveTime();
volatile unsigned long timerTime = 0, prevTime = 0;
volatile bool interrupt = false;
bool even = false;

byte digits[DIGITS_COUNT] = {
    // A - 8, G - 2, H - 1
    0b11111110, //0
    0b10110000, //1
    0b11101101, //2
    0b11111001, //3
    0b10110011, //4
    0b11011011, //5
    0b11011111, //6
    0b11110000, //7
    0b11111111, //8
    0b11111011  //9
};

void setup(){
    vw_set_rx_pin(RX_PIN);
    vw_setup(1200);
    pinMode(DATA_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(LATCH_PIN, OUTPUT);
    pinMode(SENSORPIN, INPUT);
    vw_rx_start();
    timeToDisplay(0);
    attachInterrupt(digitalPinToInterrupt(SENSORPIN), isrSaveTime, FALLING);
}

void loop(){
    uint8_t buf[VW_MAX_MESSAGE_LEN];
    uint8_t buflen = VW_MAX_MESSAGE_LEN;
    static unsigned long result;
    if(vw_get_message(buf, &buflen)){
        if((buf[0] == 'S') && (buf[1] == 'E') && (buf[2] == 'M')){
            prevTime = millis();   
            result = strtoul((char*)&buf[3], NULL, 10);
            even = !even;
        }
    }
    // if(interrupt){
    //     result = timerTime - prevTime;
    //     interrupt = false;
    // }
    if(even){
        result = millis() - prevTime;
    }
    timeToDisplay(result);
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
    byte record[DISPLAY_SIZE] = {digits[ms3Digit], digits[ms2Digit], digits[ms1Digit], digits[sec2dDigit], digits[sec1Digit], digits[minutes]};
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

void isrSaveTime(){
    unsigned long now = millis();
    if(timerTime + timerDelay < now){
        prevTime = timerTime;
        timerTime = now;
        interrupt = true;
        even = !even;
    }
}