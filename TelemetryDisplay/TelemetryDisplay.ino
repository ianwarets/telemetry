#include <VirtualWire.h>

#define DATA_PIN 6
#define CLOCK_PIN 7
#define LATCH_PIN 8
#define DIGITS_COUNT 10
#define DISPLAY_SIZE 6
#define RX_PIN 3
#define SENSORPIN 2
unsigned long timerDelay = 1500;
unsigned long showDelay = 600;
volatile unsigned long secondTime = 0;
volatile unsigned long firstTime = 0;
volatile bool evenIrq = false;
const int sensorCode = 731;
uint32_t msgCounter = 0;
bool radioEnabled = false;
bool sensorIrqEnabled = false;
// Значение для обозначения получения инициализирующего пакета от передатчика. Значение болше 10 минут в мс.
// 9 мин 59с 999 мс = 60000*9=540000 + 59999 = 599999
const unsigned long notInitialized = 700003;
const unsigned long initialized = 700002;
const unsigned long ready = 700001;
const unsigned long noSignal = 700000;
const unsigned long heartBeatInterval = 3000;

struct message {
    int code;
    unsigned int msgCounter;
    unsigned long time;
};

void setup(){
    pinMode(DATA_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(LATCH_PIN, OUTPUT);
    pinMode(SENSORPIN, INPUT_PULLUP);
    
    // Проверка подключения датчика. Погда датчик подключен, 
    // то при отсутствии лазера на входе датчика на выходе будет 1.
    // Если датчик подключен, включаем прерывания. Если нет, включаем радиоканал.
    if(digitalRead(SENSORPIN) == 1){
        //enableSensorIrq();
        showWiredEnabled();
    }else{
        vw_set_rx_pin(RX_PIN);
        vw_setup(1200);
        vw_rx_start();
        radioEnabled = true;
        showRadioEnabled();
    }
    delay(showDelay);
}

void loop(){
    unsigned long result = 0;
    if(radioEnabled){
        result = radioChannel();
    }else{
        result = wiredChannel();
    }
    switch (result){
        case noSignal:
            showNoSignal();
            //delay(showDelay);
            break;
        case initialized:
            showInitialized();
            //delay(showDelay);
            break;
        case ready:
            showReady();
            //delay(showDelay);
            break;
        case notInitialized:
            showNotInitialized();
            //delay(showDelay);
            break;
        default:
            timeToDisplay(result);
    }
}
unsigned long radioChannel(){
    message msg;
    uint8_t buflen = sizeof(msg);
    static unsigned long result = notInitialized;
    static unsigned long localFirstTime = 0;
    static unsigned long remoteFirstTime = 0;
    static unsigned long lastMessage = millis();
    if(vw_get_message((uint8_t*)&msg, &buflen)){
        if(msg.code == sensorCode){
            if(msg.msgCounter == 0 && msg.time == initialized){
                // Инициализация датчика прошла успешно
                // Показать сообщение о подключении к датчику. Вертикальные палочки заполняют экран.
                result = initialized;
                msgCounter = 0;
                remoteFirstTime = 0;
                evenIrq = false;
            }else if(msg.msgCounter > msgCounter){
                switch(msg.time){
                    case noSignal:
                        remoteFirstTime = 0;
                        evenIrq = false;
                        result = noSignal;
                        break;
                    case ready:
                        result = ready;
                        break;
                    default:
                        if(evenIrq){
                            result = msg.time - remoteFirstTime;
                        }else{
                            remoteFirstTime = msg.time;
                            localFirstTime = millis();
                        }
                        evenIrq = !evenIrq;
                }
                msgCounter = msg.msgCounter;
            }
            lastMessage = millis();
        }
    }else if(lastMessage + heartBeatInterval < millis()){
                result = notInitialized;
                msgCounter = 0;
                remoteFirstTime = 0;
                evenIrq = false;
    }else if(evenIrq){
        result = millis() - localFirstTime;
    }
    return result;
}
unsigned long wiredChannel(){
    unsigned long result = 0;
    static unsigned long lastNoSignal = 0;
    // No signal from sensor. Signal = 0. No signal = 1;
    int signal = !digitalRead(SENSORPIN);
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
        result = noSignal;
    }else{
        if(!sensorIrqEnabled){
            secondTime = 0;
            firstTime = 0;
            evenIrq = false;
            delay(1000);
            enableSensorIrq();
            sensorIrqEnabled = true;
            result = ready;
        }else{
            if(firstTime == 0){
                result = ready;
            }else{
                if(evenIrq){
                    result = millis() - firstTime;
                    if(result > 599999){
                        firstTime = millis();
                        result = 0;
                    }
                }else{
                    result = secondTime - firstTime;
                }
            }
        }
    }
    return result;
}
void showRadioEnabled(){
    byte radio = 0b01100111; //Р
    byte arr[DISPLAY_SIZE] = {0,0,0,0,0,radio};
    writeDataToDisplay(arr);
}
void showWiredEnabled(){
    byte wired = 0b01110110; // П
    byte arr[DISPLAY_SIZE] = {0,0,0,0,0,wired};
    writeDataToDisplay(arr);
}
void showReady(){
    byte zero = 0b01111110;
    byte arr[DISPLAY_SIZE] = {zero, 0, 0, 0, 0, 0};
    writeDataToDisplay(arr);
}
void showInitialized(){
    byte ii = 0b00110110; // ||
    byte data[DISPLAY_SIZE] = {ii, ii, ii, ii, ii, ii};
    writeDataToDisplay(data);
}
void showNoSignal(){
    static unsigned long lastCall = millis();
    static short symbolPosition = 1;
    static bool up = true;
    byte minus = 0b00000001; //-
    byte data[DISPLAY_SIZE] = {0,0,0,0,0,0};
    data[symbolPosition] = minus;
    if(symbolPosition == 5){
        up = false;
    }
    if(symbolPosition == 0){
        up = true;
    }
    if(lastCall + showDelay < millis()){
        if(up){    
            symbolPosition++;
        }else{
            symbolPosition--;
        }
        lastCall = millis();
    }
    writeDataToDisplay(data);
}
void showNotInitialized(){
    static unsigned long lastCall = millis();
    static short symbolPosition = 1;
    static bool up = true;
    byte bar = 0b00110110; //-
    byte data[DISPLAY_SIZE] = {0,0,0,0,0,0};
    data[symbolPosition] = bar;
    if(symbolPosition == 5){
        up = false;
    }
    if(symbolPosition == 0){
        up = true;
    }
    if(lastCall + showDelay < millis()){
        if(up){
            symbolPosition++;
        }else{
            symbolPosition--;
        }
        lastCall = millis();
    }
    writeDataToDisplay(data);
}
void timeToDisplay(unsigned long time){
    const byte digits[DIGITS_COUNT] = {
        // A - 2, G - 8, H - 1
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
    int sec_dozens = seconds/10;
    int sec_units = seconds%10;
    int ms_hundreds = milliseconds/100;
    int ms_dozens = (milliseconds%100)/10;
    int ms_units = milliseconds%10;

    byte record[DISPLAY_SIZE] = {
        digits[ms_units], 
        digits[ms_dozens], 
        digits[ms_hundreds], 
        digits[sec_units], 
        digits[sec_dozens], 
        digits[minutes]};
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
    if(!evenIrq && secondTime + timerDelay < now){
        evenIrq = !evenIrq;
        firstTime = now;
        return;      
    }
    if(evenIrq && firstTime + timerDelay < now){
        evenIrq = !evenIrq;
        secondTime = now;
        return;     
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