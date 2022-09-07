//2022/8/11　11:00 基本機能としては完成した！！
//2022/8/9 13:20 ENV3をベースに、Line送信を研究する



/*
 * 2022/8/2 
ENV3　メーカー公式のサンプルプログラムに、時計やWBGTなどの機能を追加


 
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with M5StickCPlus sample source code
*                          配套  M5StickCPlus 示例源代码
* Visit for more information: https://docs.m5stack.com/en/unit/envIII
* 获取更多资料请访问: https://docs.m5stack.com/zh_CN/unit/envIII
*
* Product: ENVIII_SHT30_QMP6988.  环境传感器
* Date: 2022/7/20
*******************************************************************************
  Please connect to Port,Read temperature, humidity and atmospheric pressure and
  display them on the display screen
  请连接端口,读取温度、湿度和大气压强并在显示屏上显示
*/
#include <M5StickCPlus.h>
#include "M5_ENV.h"

//=============================
#include <WiFiClient.h>
#include <WiFiClientSecure.h>

//#include <Wire.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

//#include <IOXhop_FirebaseESP32.h>  

#include "ArduinoJson.h"

#include <time.h>
#define JST 3600*9

WiFiClientSecure httpsClient;
//const char* WIFI_SSID = "B_IoT";
//const char* WIFI_PASSWORD = "wF7y82Az";

const char* WIFI_SSID = "Buffalo-G-3408";
const char* WIFI_PASSWORD = "fuubus6u8rfa6";

 


WiFiClientSecure client; 
// 時計の設定
//https://qiita.com/tranquility/items/5d0b1a259a0570be35ec
const char* ntpServer = "ntp.jst.mfeed.ad.jp"; // NTPサーバー
const long  gmtOffset_sec = 9 * 3600;          // 時差９時間
const int   daylightOffset_sec = 0;            // サマータイム設定なし

RTC_TimeTypeDef RTC_TimeStruct; // 時刻
RTC_DateTypeDef RTC_DateStruct; // 日付

unsigned long setuptime; // スリープ開始判定用（ミリ秒）
//unsigned long setuptime2nd; // wifi再接続不調時のリブート用（ミリ秒）

long beforeMiriSec = 0 ;

long swStartMills=0; //前回実行の時間を格納する。
long nowMillis;

long LcdMyCount=0;

int sMin = 0; // 画面書き換え判定用（分）
int sHor = 0; // 画面書き換え判定用（時）
int sDat = 0; //
//=============================


bool PIR_bool = false;
long PIR_Counter = 0;
long beforePIR_Sec = 0;

bool bl1stSen = false;
String st1stTime = "";

SHT3X sht30;
QMP6988 qmp6988;

float tmp      = 0.0;
float hum      = 0.0;
float pressure = 0.0;


  //温度データを小数点以下切り上げ
  int tmp_i = 0;
  //温度インデックス
  int index_tmp = 0;
  //湿度データを整数型に
  int hum_i = 0;
  //湿度インデックス
  int index_hum = 0;

  //WBGT表示値
  int WBGT_V = 0;

  String WBGT_Word = "";

String ThisColor = "BLACK"; 

//WBGT計算テーブル　https://symamone-tec.com/measuring_wbgt_m5stick-c/
const int WBGT[20][17] = {{15,15,16,16,17,17,18,19,19,20,20,21,21,22,23,23,24},
                          {15,16,17,17,18,18,19,19,20,21,21,22,22,23,24,24,25},
                          {16,17,17,18,19,19,20,20,21,22,22,23,23,24,25,25,26},
                          {17,18,18,19,19,20,21,21,22,22,23,24,24,25,26,26,27},
                          {18,18,19,20,20,21,22,22,23,23,24,25,25,26,27,27,28},
                          {18,19,20,20,21,22,22,23,24,24,25,26,26,27,28,28,29},
                          {19,20,21,21,22,23,23,24,25,25,26,27,27,28,29,29,30},
                          {20,21,21,22,23,23,24,25,25,26,27,28,28,29,30,30,31},
                          {21,21,22,23,24,24,25,26,26,27,28,29,29,30,31,31,32},
                          {21,22,23,24,24,25,26,27,27,28,29,29,30,31,32,32,33},
                          {22,23,24,24,25,26,27,27,28,29,30,30,31,32,33,33,34},
                          {23,24,25,25,26,27,28,28,29,30,31,31,32,33,34,34,35},
                          {24,25,25,26,27,28,28,29,30,31,32,32,33,34,35,35,36},
                          {25,25,26,27,28,29,29,30,31,32,33,33,34,35,36,37,37},
                          {25,26,27,28,29,29,30,31,32,33,33,34,35,36,37,38,38},
                          {26,27,28,29,29,30,31,32,33,34,34,35,36,37,38,39,39},
                          {27,28,29,29,30,31,32,33,34,35,35,36,37,38,39,40,41},
                          {28,28,29,30,31,32,33,34,35,35,36,37,38,39,40,41,42},
                          {28,29,30,31,32,33,34,35,35,36,37,38,39,40,41,42,43},
                          {29,30,31,32,33,34,35,35,36,37,38,39,40,41,42,43,44}};


//▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲


//Lineまわりの設定◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎
//const char* lineApiToken = "PVCmkjDuuqJlHTKflnB9pbLp4JzL2l6P3niyjyXJmnf"; //ATKMグループ
//const char* lineApiToken = "2l6LKlSW715DICCAM8D1YAmkiUS0fWwUWlcSH1P1qH1"; //単独通知

const char* lineApiToken = "TJyRhpjCqY8Kzwc3jLVcBZdih4xhHCXZAs6trx9E7Ak";//Yamazaki(AIK)グループ

//const char* lineApiToken = "LBoRUawHvBonDGG26k7pB5BokaF2hrXHEkNLCMsx9ZT"; //テスト用

// https://devdocs.line.me/files/sticker_list.pdf
//https://developers.line.biz/ja/docs/messaging-api/sticker-list/#sticker-definitions
int stickerPackage = 8515;//1
int stickerId = 16581243; //113

int stickerPackage2 = 1070;//1
int stickerId2 = 51626495; //113

int stickerPackage3 = 8515;//1
int stickerId3 = 52114113; //113
 
const char* host = "notify-api.line.me";
const String url = "/api/notify";
const int httpsPort = 443;

String MachineNo = "ENV3";  //
//◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎◎

bool blSendToLine = false;

void printLocalTime() {
  nowMillis = millis()-swStartMills;
  
  static const char *wd[7] = {"Sun","Mon","Tue","Wed","Thr","Fri","Sat"};
  M5.Rtc.GetTime(&RTC_TimeStruct); // 時刻の取り出し
  M5.Rtc.GetData(&RTC_DateStruct); // 日付の取り出し
//
//Serial.print("格納は");
//Serial.println(sDat);
//
//Serial.print("最新の日付は");
//Serial.println(RTC_DateStruct.Date);

  if (sDat != RTC_DateStruct.Date) { //日付が変わったら、カウンターを０にする
    
    LcdMyCount = 0;
    blSendToLine = false;
    bl1stSen = false;
    LcdSet(0);    
  }

  if (sHor != RTC_TimeStruct.Hours) {  //時間の時が変わったら、温度などをLINEに自動送信
    if(PIR_Counter!=0){     //PIRセンシングがあった場合のみ、LINE送信を実行する。
      GenENV();
      PIR_Counter = 0;  //PIR反応回数をリセットする。一時間あたりの反応回数で、活動量を把握する。
      bl1stSen =false;  //時が変わったら、センシング時刻のフラッグをfalseにする。これで、次回のLoopでのPIR反応時間が格納される。
    }
  }

 

  M5.Lcd.setTextSize(1);
  
  if (sMin == RTC_TimeStruct.Minutes) {
    // 秒の表示エリアだけ書き換え
      if(WBGT_V<=20){
        ThisColor = "CYAN";
        M5.Lcd.fillRect(140,5,72,25,CYAN);
        
      }else if(WBGT_V>20 && WBGT_V<=24){
  
        M5.Lcd.fillRect(140,5,72,25,GREEN);
  
      }else if(WBGT_V>24 && WBGT_V<=27){
  
        M5.Lcd.fillRect(140,5,72,25,YELLOW);
  
      }else if(WBGT_V>27 && WBGT_V<=30){
  
        M5.Lcd.fillRect(140,5,72,25,ORANGE);
  
      }else{
  
        M5.Lcd.fillRect(140,5,72,25,RED);
      }
      
  } else {
    //分が変わったら

       if(WBGT_V<=20){
    //        ThisColor = "CYAN";
            M5.Lcd.fillRect(0,0,150,60,CYAN);
            
            
          }else if(WBGT_V>20 && WBGT_V<=24){
      
            M5.Lcd.fillRect(0,0,150,60,GREEN);
      
          }else if(WBGT_V>24 && WBGT_V<=27){
      
            M5.Lcd.fillRect(0,0,150,60,YELLOW);
      
          }else if(WBGT_V>27 && WBGT_V<=30){
      
            M5.Lcd.fillRect(0,0,150,60,ORANGE);
      
          }else{
      

            M5.Lcd.fillRect(0,0,160,60,RED);
      }

      if(RTC_TimeStruct.Minutes % 10 == 0){             
          // 「分」が変わった、かつ１０分ごとに温湿度データを送信
         Adafruit();
         
      }
        //色の段階的な意味を知らしめるための、長方形インジケーター表示------
        M5.Lcd.fillRect(0,60,55,20,CYAN);
        M5.Lcd.fillRect(50,60,55,20,GREEN);
        M5.Lcd.fillRect(100,60,55,20,YELLOW);
        M5.Lcd.fillRect(150,60,55,20,ORANGE);
        M5.Lcd.fillRect(200,60,55,20,RED);
        //---------------------------------------------------------
    }



  
    M5.Lcd.setTextColor(BLACK);
    M5.Lcd.setCursor(0, 10, 7);  //x,y,font 7:48ピクセル7セグ風フォント
  
    M5.Lcd.printf("%02d:%02d", RTC_TimeStruct.Hours, RTC_TimeStruct.Minutes); // 時分を表示
    
    M5.Lcd.setTextFont(1); // 1:Adafruit 8ピクセルASCIIフォント
    M5.Lcd.setTextSize(2);
  
    M5.Lcd.printf(":%02d\n",RTC_TimeStruct.Seconds); // 秒を表示

    
    //日付の表示^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    M5.Lcd.setCursor(5, 63, 1);  //x,y,font 1:Adafruit 8ピクセルASCIIフォント
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(TFT_DARKGREY);
  //  M5.Lcd.printf("D:%04d.%02d.%02d %s\n", RTC_DateStruct.Year, RTC_DateStruct.Month, RTC_DateStruct.Date, wd[RTC_DateStruct.WeekDay]);
    M5.Lcd.printf("%04d.%02d.%02d %s\n", RTC_DateStruct.Year, RTC_DateStruct.Month, RTC_DateStruct.Date, wd[RTC_DateStruct.WeekDay]);
    //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  
    sMin = RTC_TimeStruct.Minutes; // 「分」を保存
    sHor = RTC_TimeStruct.Hours; // 「時」を保存
    sDat = RTC_DateStruct.Date ; //日付を保存


//=就業時間外や休憩時間はカウントしないようにする========================================
//  bool restTime = false;
//
//  if(sHor <= 7){
//    restTime = true;
//  }
//  
//  if(sHor == 10 && sMin >=5 && sMin <=9){
//
//    restTime = true;
//  }else{
////    Serial.println("❒❒❒");
//  }
//
//  if(sHor == 12 && sMin >=0 && sMin <=49){
//    restTime = true;
//  }else{
//  }
//
//  if(sHor == 15 && sMin >=5 && sMin <=9){
//    restTime = true;
//  }else{
//  }


  
//=======================================================================


//
//  M5.Lcd.setTextSize(2);//文字の大きさを設定（1～7）
//
//  M5.Lcd.setCursor(190, 10, 1);

//  //#################################################################################################
}

//=======================================================================================

void LcdSet(int LcdMyCount){
// カウント表示の更新=============================================================

    //*****画面描画 start *********************************************
    if(WBGT_V<=20){

      M5.Lcd.fillRect(160, 30, 50, 30, CYAN);
    }else if(WBGT_V>20 && WBGT_V<=24){

      M5.Lcd.fillRect(160, 30, 50, 30, GREEN);

    }else if(WBGT_V>24 && WBGT_V<=27){

     M5.Lcd.fillRect(160, 30, 50, 30, YELLOW);

    }else if(WBGT_V>27 && WBGT_V<=30){

      M5.Lcd.fillRect(160, 30, 50, 30, ORANGE);

    }else{

      M5.Lcd.fillRect(160, 30, 50, 30, RED);

    }
    
    M5.Lcd.setCursor(160, 35,1);
    if(LcdMyCount == 0 ){
       M5.Lcd.setTextColor(PINK); //文字色設定(背景は透明)(WHITE, BLACK, RED, GREEN, B
    }else{
       M5.Lcd.setTextColor(BLUE); //文字色設定(背景は透明)(WHITE, BLACK, RED, GREEN, B
    }
  //  M5.Lcd.printf(LcdMyCount);
    M5.Lcd.setTextSize(3);      // 文字サイズを3にする
  //  M5.Lcd.printf("c:%1.0fc\r\n", LcdMyCount);
    M5.Lcd.print(LcdMyCount);
    M5.Lcd.setTextSize(1);

//===============================================================================


}

void Adafruit(){
//  Serial.println("Adaに入りました");

  pressure = qmp6988.calcPressure();
  if (sht30.get() == 0) {  // Obtain the data of shT30. 
      tmp = sht30.cTemp;   // Store the temperature obtained from shT30.

      tmp_i = (tmp-0.1) + 1;
  } else {
      tmp = 0, hum = 0;
  }
    

    //温度インデックス
    if(tmp_i<=20){    //マイナスになると、おかしな数字になるので、強制的に０とする。
      index_tmp = 0 ;
    }else{
      index_tmp = tmp_i - 21;
    }
    
 
    hum = sht30.humidity;
    //湿度データを整数型に
    hum_i = hum;
    //湿度インデックス
    index_hum = hum / 5 - 3;

    

     //WBGT表示値
    WBGT_V = WBGT[index_tmp][index_hum];

//    sendToFirebaseENV(tmp,hum,pressure / 100);

    M5.Lcd.setTextColor(BLACK);
    //*****画面描画 start *********************************************
    if(WBGT_V<=20){

      M5.Lcd.fillScreen(CYAN);
      WBGT_Word = "青";
    }else if(WBGT_V>20 && WBGT_V<=24){

      M5.Lcd.fillScreen(GREEN);
      WBGT_Word = "緑";
    }else if(WBGT_V>24 && WBGT_V<=27){

      M5.Lcd.fillScreen(YELLOW);
      WBGT_Word = "注意 黄";
    }else if(WBGT_V>27 && WBGT_V<=30){

      M5.Lcd.fillScreen(ORANGE);
      WBGT_Word = "警戒 橙";
    }else{

      M5.Lcd.fillScreen(RED);
      M5.Lcd.setTextColor(WHITE);
      WBGT_Word = "危険 赤";
    }
  //*****画面描画 end**************************************

    M5.Lcd.setTextSize(2);      // 文字サイズを2にする
    M5.Lcd.setCursor(0, 85, 1);
    
    M5.Lcd.printf("Temp: %4.1f'C\r\n", tmp);

    M5.Lcd.printf("Humid:%4.1f%%\r\n", hum);

    M5.Lcd.printf("WBGT:%4.0d'C\r\n", WBGT_V);

    M5.Lcd.setTextSize(1);      // 文字サイズを2にする
    delay(10);

    LcdSet(0);

   
}


void wifiConnect(){
  M5.Lcd.setRotation(3);
  M5.Lcd.setCursor(0, 0, 1);
  M5.Lcd.fillScreen(BLACK);
  
  int cnt=0;
  M5.Lcd.printf("Connecting to %s\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  while(WiFi.status() != WL_CONNECTED) {
    cnt++;
    delay(500);
    M5.Lcd.print(".");
    if(cnt%10==0) {
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      M5.Lcd.println("");
    }
    if(cnt>=60) {
      ESP.restart();
    }
  }

  M5.Lcd.printf("\nWiFi connected\n");
  delay(1000);
  M5.Lcd.fillScreen(BLACK);
}
//◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆


void lineNotify2nd(String message, int stkpkgid, int stkid){
  //digitalWrite(ledPin, HIGH);
 
  Serial.print("connecting to ");
  Serial.println(host);
 
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
/*  

*/
  Serial.print("requesting URL: ");
  Serial.println(url);
  String lineMessage = "message=" + message;
 
  if((stkid != 0) && (stkpkgid != 0)){
    lineMessage += "&stickerPackageId=";
    lineMessage += stkpkgid;
    lineMessage += "&stickerId=";
    lineMessage += stkid;
  }
 
  String header = "POST " + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: ESP8266\r\n" +
               "Authorization: Bearer " + lineApiToken + "\r\n" +
               "Connection: close\r\n" +
               "Content-Type: application/x-www-form-urlencoded\r\n" +
               "Content-Length: " + lineMessage.length() + "\r\n";
 
  client.print(header);
  client.print("\r\n");
  client.print(lineMessage + "\r\n");
  
  Serial.print("request sent:");
  Serial.println(lineMessage);
  
  String res = client.readString();
  Serial.println(res);
  client.stop();
}


void GenENV(){
      String MyANP2 = String(PIR_Counter);

      String WBGT_Word2 = WBGT_Word + WBGT_V;
      Serial.println(WBGT_Word);
      Serial.println(MyANP2);
      char msg[100];

      sprintf(msg, "\r\n気温: %2.0f 'C\r\n湿度:%2.0f %\r\nWBGT:%6s\r\n回数:%10s\r\n時刻:%10s", tmp,hum,WBGT_Word2,MyANP2,st1stTime);//８月１１日LINE送信大成功！！！

      int MyRand = 16581265 - random(23);
      lineNotify2nd(msg, stickerPackage3, MyRand);
}

void setup() {
    Serial.begin(9600);
    client.setInsecure();
    M5.begin();             // Init M5StickCPlus.  初始化M5StickCPlus
    M5.Lcd.setRotation(3);  // Rotate the screen.  旋转屏幕

    pinMode(36,INPUT_PULLUP);
    
    wifiConnect();
    configTime( JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");
// Get local time
    struct tm timeInfo;
    if (getLocalTime(&timeInfo)) {
      M5.Lcd.print("NTP : ");
      M5.Lcd.println(ntpServer);

      // Set RTC time
      RTC_TimeTypeDef TimeStruct;
      TimeStruct.Hours   = timeInfo.tm_hour;
      TimeStruct.Minutes = timeInfo.tm_min;
      TimeStruct.Seconds = timeInfo.tm_sec;
      M5.Rtc.SetTime(&TimeStruct);

      RTC_DateTypeDef DateStruct;
      DateStruct.WeekDay = timeInfo.tm_wday;
      DateStruct.Month = timeInfo.tm_mon + 1;
      DateStruct.Date = timeInfo.tm_mday;
      DateStruct.Year = timeInfo.tm_year + 1900;
      M5.Rtc.SetData(&DateStruct);
    }

  time_t t;
  t = time(NULL);
//  client.setInsecure();
  
    Wire.begin(32,33);  // Wire init, adding the I2C bus.  
    qmp6988.init();
    M5.lcd.println(F("ENVIII Unit(SHT30 and QMP6988) test"));
   Adafruit();
   printLocalTime();
   LcdSet(0);

}


void loop() {
    delay(10);

    // ボタンの状態を更新
    M5.update();
  
    if(millis()>beforeMiriSec+1000){
        //一秒ごとに実行される、時刻更新処理
        printLocalTime();
        beforeMiriSec = millis();
    }

    if(PIR_Counter==0 || millis()>beforePIR_Sec + 60000){ //６０秒あける

//      Serial.println("入ったよ");
//      Serial.println(PIR_Counter);
      if(digitalRead(36)){              //PIR を検出
        Serial.println("反応したよ");
        PIR_Counter ++;

       if(bl1stSen ==false){
        char hms[100];
        M5.Rtc.GetTime(&RTC_TimeStruct); // 時刻の取り出し
        sprintf(hms, "%02d:%02d:%02d\n", RTC_TimeStruct.Hours, RTC_TimeStruct.Minutes,RTC_TimeStruct.Seconds); // 時分を表示

          st1stTime = hms;
          bl1stSen = true;
          Serial.println(st1stTime);
      }
        
        M5.Lcd.fillRect(170,85,60,40,BLACK);
        M5.Lcd.setCursor(180,95);
        M5.Lcd.setTextSize(3);     
        M5.Lcd.setTextColor(GREEN);
        M5.Lcd.print(PIR_Counter); 
        beforePIR_Sec = millis();       //センシングの時間を格納して６０秒のインターバルの起点時間とする。
      }
      
    }
    

    if(M5.BtnA.wasPressed()){
      GenENV();
     
    }
    


    
}
