#include <VirtualWire.h>

#define DATA_PIN 6
#define CLOCK_PIN 7
#define LATCH_PIN 8
#define DIGITS_COUNT 10
#define DISPLAY_SIZE 6
#define RX_PIN 3
#define SENSORPIN 2
unsigned long timerDelay = 1500;
unsigned long showDelay = 1000;
void isrSaveTime();
volatile unsigned long secondTime = 0, firstTime = 0;
volatile bool interrupt = false;
bool even = false;
int sensorCode = 731;
uint32_t msgCounter = 0;
bool radioEnabled = false;
bool sensorIrqEnabled = false;
// Значение для обозначения получения инициализирующего пакета от передатчика. Значение болше 10 минут в мс.
const unsigned long initialized = 1;
const unsigned long ready = 2;

struct message {
    int code;
    unsigned int msgCounter;
    unsigned long time;
};

void setup(){
    pinMode(DATA_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(LATCH_PIN, OUTPUT);
    pinMode(SENSORPIN, INPUT);
    
    // Проверка подключения датчика. Погда датчик подключен, 
    // то при отсутствии лазера на входе датчика на выходе будет 1.
    // Если датчик подключен, включаем прерывания. Если нет, включаем радиоканал.
    if(digitalRead(SENSORPIN) == 1){
        enableSensorIrq();
    }else{
        vw_set_rx_pin(RX_PIN);
        vw_setup(1200);
        vw_rx_start();
        radioEnabled = true;
    }
}

void loop(){
    unsigned long result = 0;
    if(radioEnabled){
        result = radioChannel();
    }else{
        result = wiredChannel();
    }
    switch (result){
        case 0:
            showNoSignal();
            break;
        case initialized:
            showInitialized();
            break;
        case ready:
            showReady();
            break;
        default:
            timeToDisplay(result);
    }
    delay(showDelay);
}
unsigned long radioChannel(){
    message msg;
    uint8_t buflen = sizeof(msg);
    unsigned long result;
    static bool radioInitialized = false;
    static unsigned long localFirstTime = 0;
    if(vw_get_message((uint8_t*)&msg, &buflen)){
        if(msg.code == sensorCode){
            if(msg.msgCounter == 0){
                // Инициализация датчика прошла успешно
                // Показать сообщение о подключении к датчику. Вертикальные палочки заполняют экран.
                radioInitialized = true;
                return initialized;
            }else{
                if(msg.msgCounter == msgCounter + 1){
                    if(even){
                        result = firstTime - msg.time;

                    }else{
                        firstTime = msg.time;
                        localFirstTime = millis();
                    }
                    even = !even;
                    msgCounter = msg.msgCounter;
                }
            }
        }
    }
    if(even){
        return result;
    }else{
        return millis() - localFirstTime;
    }
}
unsigned long wiredChannel(){
    unsigned long result = 0;
    // No signal from sensor. Signal = 0. No signal = 1;
    if(digitalRead(SENSORPIN) == 1){
        disableSensorIrq();
    }else{
        if(!sensorIrqEnabled){
            enableSensorIrq();
        }else{
            if(interrupt){
                result = millis() - secondTime;
                interrupt = false;
            }
            if(even){
                result = secondTime - firstTime;
            }
        }
    }
    return result;
}
void showReady(){
    byte data[2] = {
        0b00000000,
        0b11111110
    };
    byte arr[DISPLAY_SIZE] = {data[0], data[0], data[0], data[0], data[0], data[1]};
    writeDataToDisplay(arr);
}
void showInitialized(){
    byte ii = 0b00110110; // ||
    byte data[DISPLAY_SIZE] = {ii, ii, ii, ii, ii, ii};
    writeDataToDisplay(data);
}
void showNoSignal(){
    byte minus = 0b00000001;
    byte data[DISPLAY_SIZE] = {minus, minus, minus, minus, minus, minus};
    writeDataToDisplay(data);
}
void timeToDisplay(unsigned long time){
    const byte digits[DIGITS_COUNT] = {
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
    writeDataToDisplay(record);
}
void writeDataToDisplay(byte b[DISPLAY_SIZE]){
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
    if(secondTime + timerDelay < now){
        firstTime = secondTime;
        secondTime = now;
        interrupt = true;
        even = !even;
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