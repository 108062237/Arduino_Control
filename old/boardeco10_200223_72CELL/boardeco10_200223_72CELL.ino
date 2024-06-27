// 此版　溫度感測更換PT100
// 使用電流控風扇轉速
// 精進保護機制
// 電流SENSOR 使用 ACS758100U
//  增加 LCD 顯示

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define zero_current 0.6
#define A 0.06

const int buttonPin = 2;      // 按鈕(pushbutton)
const int highsideswitch = 3; // 高側開關
const int relayPin = 4;       // 開關繼電器(Relay)
const int relay = 5;          // 電磁閥繼電器
const int fanPin = 6;         // 風扇
const int buzzPin = 8;        // 蜂鳴器
const int transferPin = 11;   // RS485 PORT
unsigned int i;               // 蜂鳴器變數
unsigned int j;               // 蜂鳴器變數
unsigned long time_now = 0;
int relayState = 0; // 繼電器狀態
float disp;         // display value
int dis;
int valu;
float volt;
int vot;
float volt1;
int vot1;
float ave;
int av;
float power;
int po;

int buttonState;

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

// 設定電磁閥開啟與間隔時間
const int valveopentime = 1000;    // 設定電磁閥開啟時間
const long valveclosetime = 11000; // 設定電磁閥開啟間隔時間
const int datacatchtime = 2000;    // 數據擷取時間
// 設定解負載參數設定
const int valveVOLT = 65;
const int valveTEMP = 32;

// 設定燃料電池堆保護參數
const int lowvoltval = 36;  // 低電壓保護參數設定
const int hightempval = 45; // 過溫度保護參數設定
const int highampval = 15;  // 過電流保護參數設定
const int overpower = 450;  // 過功率保護參數設定

unsigned long time_previous = 0;   // 用於計算時間的暫存變數
unsigned long time_previous_1 = 0; // 用於計算時間的暫存變數
unsigned long time_previous_2 = 0; // 用於計算時間的暫存變數

unsigned long mybuffer[9]; // 設定暫存器的大小

long all_Timer; // millis()裡的時間，讓全域皆可使用

void setup()
{
    pinMode(transferPin, OUTPUT);    // D2
    pinMode(buttonPin, INPUT);       // 把 buttonPin 設置成 INPUT
    pinMode(fanPin, OUTPUT);         // 把fanPin設置為OUTPUT   *
    pinMode(relay, OUTPUT);          // relay設置為OUTPUT
    pinMode(relayPin, OUTPUT);       // 把 relayPin 設置成 OUTPUT
    pinMode(highsideswitch, OUTPUT); // 把HSS 設置成 OUTPUT
    pinMode(buzzPin, OUTPUT);        // 把 buzz 設置成 OUTPUT
    Serial.begin(9600);

    lcd.begin(); // initialize the lcd
    lcd.backlight();
    lcd.print("Eco Energy Tec "); // Print a message to the LCD.
    lcd.setCursor(0, 1);
    lcd.print("FC Controller");
}

void switchRelay()
{
    if (relayState == 1)
        relayState = 0; // 把繼電器狀態改為 ON
    else
        relayState = 1;                 // 把繼電器狀態改為 OFF
    digitalWrite(relayPin, relayState); // 讓繼電器作動, 切換開關
}
void valve()
{
    if (all_Timer - time_previous > valveopentime)
    {                           // 當前的時間，如果超過暫存器 200 (即 0.2S)則執行以下程式
        digitalWrite(relay, 0); // turn the LED on (HIGH is the voltage level)
        // delay(500);               // wait for a second
        // Serial.print("OPEN");
        time_previous += valveopentime; // 執行後則將暫存器數值往上累加相同的單位
    }
    if (all_Timer - time_previous_1 > valveclosetime)
    {                           // 當前的時間，如果超過暫存器 200 (即 0.2S)則執行以下程式
        digitalWrite(relay, 1); // turn the LED off by making the voltage LOW
                                // delay(15000);  // wait for a second
        time_previous_1 += valveclosetime;
        // Serial.print("Close");
    }
}

void voltage()
{
    double val = analogRead(A0);
    double vol = 0;
    for (int p = 0; p < 1000; p++)
    {
        vol = vol + (val / 65.2) * 5;
        volt = vol / 1000;
        vot = vol / 100;
    }
    // Serial.print(volt);
}
void current()
{
    float average = 0;
    double analog = analogRead(A1);
    if (analog < 130)
    {
        ave = 0;
    }
    else
    {
        for (int o = 0; o < 1000; o++)
        {
            average = average + ((0.0049 / A) * analog - (zero_current / A));
            ave = average / 1000;
            av = average / 100;
        }
        // Serial.print("    ");
        // Serial.print(ave);
        // delay(1000);
    }
}
void watt()
{
    power = volt * ave;
    po = power * 10;
}
void temp()
{
    float temp = 0;
    int value = analogRead(A2);
    // for(int a = 0; a < 1000; a++) {
    temp = ((value / 1023.0) * 500);
    disp = temp - 60;
    dis = temp * 10;
    delay(10);
    //}
    // Serial.print("    ");
    // Serial.print(disp);
    // Serial.println();
    // delay(500);
}

void fancontrol()
{
    if (ave < 1)
    { // 溫度小於等於30度：風扇轉速50
        analogWrite(fanPin, 120);
    }
    if (ave >= 1 && ave <= 5)
    {
        analogWrite(fanPin, 150);
    }
    if (ave > 5 && ave <= 10)
    { // 溫度大於等於41度；風扇轉速200
        analogWrite(fanPin, 170);
    }
    if (ave > 10 && ave <= 12)
    { // 溫度大於等於41度；風扇轉速200
        analogWrite(fanPin, 220);
    }
    if (ave > 12)
    { // 溫度大於等於41度；風扇轉速200
        analogWrite(fanPin, 255);
    }
    if (disp > 40)
    { // 溫度大於31度小於40度;風扇轉速150
        analogWrite(fanPin, 255);
    }
    if (disp > valveTEMP && volt > valveVOLT)
    {
        analogWrite(fanPin, 255);
    }
    delay(1);
}

void showdata()
{
    if (all_Timer - time_previous_2 > datacatchtime)
    { // 當前的時間，如果超過暫存器 200 (即 0.2S)則執行以下程式
        lcd.begin();
        lcd.print(volt); // Print a message to the LCD.
        lcd.print("V");
        lcd.print(ave); // Print a message to the LCD.
        lcd.print("A");
        lcd.setCursor(0, 1);
        lcd.print(disp);
        lcd.print("Temp");
        lcd.print(power);
        lcd.print("W");
        time_previous_2 += datacatchtime; // 執行後則將暫存器數值往上累加相同的單位
    }
}

void hss()
{
    if (relayState == 1)
        digitalWrite(highsideswitch, HIGH);
    else
        digitalWrite(highsideswitch, LOW);
}
void buzz()
{
    for (i = 0; i < 250; i++)
    {
        digitalWrite(buzzPin, HIGH); // 发声音
        delay(1);                    // 延时1ms
        digitalWrite(buzzPin, LOW);  // 不发声音
        delay(1);                    // 延时ms
    }
}

void buzz_1()
{
    for (i = 0; i < 250; i++) // 辒出一个频率的声音
    {
        digitalWrite(buzzPin, HIGH); // 发声音
        delay(1);                    // 延时1ms
        digitalWrite(buzzPin, LOW);  // 不发声音
        delay(1);                    // 延时ms
    }
    for (i = 0; i < 50; i++) // 辒出另一个频率癿声音
    {
        digitalWrite(buzzPin, HIGH); // 发声音
        delay(2);                    // 延时2ms
        digitalWrite(buzzPin, LOW);  // 不发声音
        delay(2);                    // 延时2ms
    }
}

void buzz_2()
{
    for (i = 0; i < 50; i++) // 辒出一个频率的声音
    {
        digitalWrite(buzzPin, HIGH); // 发声音
        delay(2);                    // 延时1ms
        digitalWrite(buzzPin, LOW);  // 不发声音
        delay(2);                    // 延时ms
    }
    for (i = 0; i < 250; i++) // 辒出另一个频率癿声音
    {
        digitalWrite(buzzPin, HIGH); // 发声音
        delay(1);                    // 延时2ms
        digitalWrite(buzzPin, LOW);  // 不发声音
        delay(1);                    // 延时2ms
    }
}
void buzz_3()
{
    for (i = 0; i < 10; i++) // 辒出一个频率的声音
    {
        digitalWrite(buzzPin, HIGH); // 发声音
        delay(1);                    // 延时1ms
        digitalWrite(buzzPin, LOW);  // 不发声音
        delay(1);                    // 延时ms
    }
    for (i = 0; i < 200; i++) // 辒出另一个频率癿声音
    {
        digitalWrite(buzzPin, HIGH); // 发声音
        delay(2);                    // 延时2ms
        digitalWrite(buzzPin, LOW);  // 不发声音
        delay(2);                    // 延时2ms
    }
    for (i = 0; i < 100; i++) // 辒出另一个频率癿声音
    {
        digitalWrite(buzzPin, HIGH); // 发声音
        delay(1);                    // 延时2ms
        digitalWrite(buzzPin, LOW);  // 不发声音
        delay(1);                    // 延时2ms
    }
}
int volt_status = 0; // 宣告電壓狀態起始值
long volt_temp;      // 宣告時間暫存變數

void voltlow()
{
    voltage();
    if (volt >= lowvoltval)
    {
        volt_status = 0;
    }
    else if ((volt < lowvoltval) && volt_status == 0)
    { // 當 temp 偵測超過 11 ，且狀態為 0 的時候(偵測到 11 之前)，執行以下動作
        volt_status = 1;
        volt_temp = all_Timer;
        // Serial.print("Start= ");
        // Serial.println(volt_temp);
    }

    if (volt_status == 1 && (all_Timer - volt_temp > 5000)) // 開始計時 5S ，且狀態為 1 的時候(偵測到 20A 之後)
    {
        // Serial.print("End= ");
        //  Serial.println(all_Timer);
        digitalWrite(relayPin, LOW);
        digitalWrite(relay, LOW);
        digitalWrite(highsideswitch, LOW);
        relayState = 0;
        volt_status = 0; // 改變溫度狀態為 0
        buzz_2();
    }
}

int temp_status = 0; // 設定 temp 偵測到 50 之前與之後的狀態變數
long temp_temp;      // 宣告時間暫存變數

void temphigh()
{
    temp();
    if (disp <= hightempval)
    {
        temp_status = 0;
    }
    else if (disp > hightempval && temp_status == 0)
    {
        temp_status = 1;       // 改變狀態為 1
        temp_temp = all_Timer; // 儲存偵測到
        // Serial.print("Start= ");
        // Serial.println(time_temp);
    }
    if (temp_status == 1 && (all_Timer - temp_temp > 5000)) // 開始計時 5S ，且狀態為 1 的時候(偵測到 50 之後)
    {
        digitalWrite(relayPin, LOW);
        digitalWrite(relay, LOW);
        digitalWrite(highsideswitch, LOW);
        relayState = 0;
        temp_status = 0; // 改變狀態為 0
        buzz_1();
    }
}

int amp_status = 0; // 設定 amp 偵測到 5 之前與之後的狀態變數
long amp_temp;      // 宣告時間暫存變數

void amphigh()
{
    current();
    if (ave <= highampval)
    {
        amp_status = 0;
    }
    else if (ave > highampval && amp_status == 0) // 當 amp 偵測超過 20A ，且狀態為 0 的時候(偵測到 20A 之前)，執行以下動作
    {
        amp_status = 1;       // 改變狀態為 1
        amp_temp = all_Timer; // 儲存偵測到 15A 時的時間點
        // Serial.print("Start= ");
        // Serial.println(time_temp);
    }
    if (amp_status == 1 && (all_Timer - amp_temp > 5000)) // 開始計時 5S ，且狀態為 1 的時候(偵測到 20A 之後)
    {
        // Serial.print("End= ");
        //  Serial.println(amp_Timer);
        digitalWrite(relayPin, LOW);
        digitalWrite(relay, LOW);
        digitalWrite(highsideswitch, LOW);
        relayState = 0;
        amp_status = 0; // 改變狀態為 0
        buzz_3();
    }
    delay(1);
}
int watt_status = 0; // 設定 amp 偵測到 5 之前與之後的狀態變數
long watt_temp;      // 宣告時間暫存變數

void highpower()
{
    watt();
    if (power <= overpower)
    {
        watt_status = 0;
    }
    else if (power >= overpower && watt_status == 0) // 當 amp 偵測超過 20A ，且狀態為 0 的時候(偵測到 20A 之前)，執行以下動作
    {
        watt_status = 1;       // 改變狀態為 1
        watt_temp = all_Timer; // 儲存偵測到 15A 時的時間點
        // Serial.print("Start= ");
        // Serial.println(time_temp);
    }
    if (watt_status == 1 && (all_Timer - watt_temp > 5000)) // 開始計時 5S ，且狀態為 1 的時候(偵測到 20A 之後)
    {
        // Serial.print("End= ");
        //  Serial.println(amp_Timer);
        digitalWrite(relayPin, LOW);
        digitalWrite(relay, LOW);
        digitalWrite(highsideswitch, LOW);
        relayState = 0;
        watt_status = 0; // 改變狀態為 0
        buzz_3();
    }
    delay(1);
}
void start()
{
    digitalWrite(relay, HIGH);
    delay(2000);
    digitalWrite(relay, LOW);
    delay(3000);
    digitalWrite(relay, HIGH);
    delay(2000);
    digitalWrite(relay, LOW);
    delay(3000);
    digitalWrite(relay, HIGH);
    delay(2000);
    digitalWrite(relay, LOW);
    buzz();
}

void senddata()
{ // 數據擷取顯示程式碼
    voltage();
    // LIvoltage();
    current();
    temp();
    watt();
    switch (Serial.read())
    {
    case '1': // 當偵測到5字元執行以下程式
        mybuffer[0] = 1;
        mybuffer[1] = vot >> 8;
        mybuffer[2] = vot;
        mybuffer[3] = av >> 8;
        mybuffer[4] = av;
        mybuffer[5] = dis >> 8;
        mybuffer[6] = dis;
        mybuffer[7] = po >> 8;
        mybuffer[8] = po;

        digitalWrite(transferPin, HIGH);
        delay(50);
        for (int i; i < 9; i++)
        {
            delay(1);
            Serial.write(mybuffer[i]);
            delay(1);
        }
        digitalWrite(transferPin, LOW);
        break;
    }
}
void loop()
{
    unsigned long now_time = millis(); // millis()函數只能在loop裡面宣告
    all_Timer = now_time;              // 程式開始之後，將millis()時間轉移到全域變數

    buttonState = digitalRead(buttonPin); // 檢查按鈕是否被按下(pressed)
    if (buttonState == HIGH)
    { // 有的話，buttonState 會是 HIGH

        switchRelay();
        start();
        hss();
    }
    if (relayState == 1)
    {
        // showdata();
        // senddata();
        fancontrol();
        voltage();
        current();
        temp();
        watt();
        valve();
        voltlow();
        // temphigh();
        amphigh();
        // highpower();
    }
}
