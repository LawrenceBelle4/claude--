#include <Arduino.h>

#include <SPI.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_GFX.h>

unsigned int CC = 0;
int gState = 0;
int gPrev = (-1);
int gFrm = 0;
int gMut = 0;
int gBtn = 0;
unsigned long gSolT = 0;
unsigned int gIdleSt = 0;
int gIdleSub = 0;
unsigned long gIdleT = 0;
int gTrans = 0;

Adafruit_ST7735 tft = Adafruit_ST7735(10, 9, 7, 6, 8);

void setup() {
  Serial.begin(115200);
  tft.initR(INITR_BLACKTAB);
  // 如果显示异常，可尝试以下初始化参数：
  // tft.initR(INITR_REDTAB);
  // tft.initR(INITR_GREENTAB);
  // tft.initR(INITR_144GREENTAB);
  pinMode(3, OUTPUT);

  tft.setRotation(1);
  tft.fillScreen(tft.color565(0, 0, 0));
  CC = tft.color565(222, 136, 109);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), []{ gBtn = 1; }, FALLING);
  tft.setTextSize(1); tft.setTextColor(CC);
  tft.setCursor(50, 35); tft.print("Clawd");
  tft.setCursor(45, 48); tft.print("Monitor");
  delay(1200);

}

void loop() {

  if (Serial.available() >= 1) {
    String s = Serial.readStringUntil('\n'); s.trim();
    if(s=="idle") gState=0;
    else if(s=="thinking") gState=1;
    else if(s=="coding") gState=2;
    else if(s=="done") gState=3;
    else if(s=="permission") gState=4;
  }
  if (gBtn == 1) {
    gBtn=0; delay(40);
    if(digitalRead(2)==LOW){
      unsigned long t0=millis();
      while(digitalRead(2)==LOW && millis()-t0<1200) delay(20);
      if(millis()-t0>700){gMut=!gMut; Serial.println(gMut?"muted":"unmuted");}
      else{Serial.println("btn"); if(!gMut) tone(4,1000,50);}
    }
  }
  if (gState != gPrev) {
    gPrev=gState; gFrm=0;
    if(gState==0){gIdleSt=0; gIdleT=millis();}
    else{gTrans=(gState==1)?(int)gIdleSt:0; gIdleSt=0;}
    if(gState==3){digitalWrite(3,HIGH);delay(100);digitalWrite(3,LOW);delay(100);digitalWrite(3,HIGH);delay(100);digitalWrite(3,LOW);}
    if(gState==4){for(int i=0;i<4;i++){digitalWrite(3,HIGH);delay(40);digitalWrite(3,LOW);delay(40);}}
  }
  if (gState == 0) {
    static unsigned long t=0;
    int dl=(gIdleSt==0)?300:(gIdleSt==1)?500:600;
    if(millis()-t>(unsigned long)dl){
      t=millis();
      if(gIdleSt==0){
        gFrm=(gFrm+1)%20;
        if(millis()-gIdleT>60000){gIdleSt=random(2)+1;gIdleSub=0;}
      }else{
        gIdleSub=(gIdleSub+1)%20;
      }
      tft.fillScreen(0);
      if(gIdleSt==0){
        int yO=(gFrm%6<3)?0:1;
        bool blink=(gFrm%20>=16&&gFrm%20<=17);
        tft.fillRect(71,46,18,2,0x4208);
        tft.fillRect(71,38,2,8,CC);tft.fillRect(75,38,2,8,CC);
        tft.fillRect(83,38,2,8,CC);tft.fillRect(87,38,2,8,CC);
        tft.fillRect(69,28+yO,22,14,CC);
        tft.fillRect(65,34+yO,4,4,CC);tft.fillRect(91,34+yO,4,4,CC);
        if(!blink){tft.fillRect(73,32+yO,2,4,0);tft.fillRect(85,32+yO,2,4,0);}
        tft.setTextSize(1);tft.setTextColor(0x04DF);tft.setCursor(5,70);tft.print("idle");
      }else if(gIdleSt==1){
        int yO=(gIdleSub%8<4)?0:1;
        uint16_t bC=tft.color565(139,69,19);
        uint16_t sB=tft.color565(100,50,10);
        tft.fillRect(71,47,18,2,0x4208);
        tft.fillRect(71,40,2,5,CC);tft.fillRect(75,40,2,5,CC);
        tft.fillRect(83,40,2,5,CC);tft.fillRect(87,40,2,5,CC);
        tft.fillRect(69,30+yO,22,12,CC);
        tft.fillRect(65,38+yO,4,3,CC);tft.fillRect(91,38+yO,4,3,CC);
        tft.fillRect(73,34+yO,2,2,0);tft.fillRect(85,34+yO,2,2,0);
        tft.fillRect(68,39+yO,11,7,bC);tft.fillRect(81,39+yO,11,7,bC);
        tft.fillRect(69,40+yO,9,5,0xFFFF);tft.fillRect(82,40+yO,9,5,0xFFFF);
        tft.fillRect(79,39+yO,2,7,sB);
        int tl=gIdleSub%6;
        tft.fillRect(70,41+yO,7,1,0);tft.fillRect(70,43+yO,5,1,0);
        tft.fillRect(83,41+yO,4+tl,1,0);tft.fillRect(83,43+yO,3+((tl+2)%5),1,0);
        if(gIdleSub%10>=8) tft.fillRect(79,39+yO,4,7,0xFFFF);
        tft.setTextSize(1);tft.setTextColor(0xFFE0);tft.setCursor(5,70);tft.print("reading");
      }else{
        int yO=(gIdleSub%10<5)?0:1;
        tft.fillRect(71,46,18,2,0x4208);
        tft.fillRect(71,38,2,8,CC);tft.fillRect(75,38,2,8,CC);
        tft.fillRect(83,38,2,8,CC);tft.fillRect(87,38,2,8,CC);
        tft.fillRect(69,28+yO,22,14,CC);
        tft.fillRect(65,34+yO,4,4,CC);tft.fillRect(91,34+yO,4,4,CC);
        tft.fillRect(73,34+yO,2,1,0);tft.fillRect(85,34+yO,2,1,0);
        int zp=gIdleSub%10;
        tft.setTextSize(1);tft.setTextColor(0x07FF);
        tft.setCursor(94,24-zp);tft.print("z");
        if(zp>=3){tft.setCursor(100,18-zp);tft.print("z");}
        if(zp>=6){tft.setCursor(106,12-zp);tft.print("z");}
        tft.setTextSize(1);tft.setTextColor(0x8410);tft.setCursor(5,70);tft.print("sleeping");
      }
    }
  }
  if (gState == 1) {
    static unsigned long t=0;
    if(millis()-t>500){
      t=millis();
      tft.fillScreen(0);
      if(gTrans>0 && gFrm<4){
        gFrm++;
        if(gTrans==1){
          tft.fillRect(71,46,18,2,0x4208);
          tft.fillRect(71,38,2,8,CC);tft.fillRect(75,38,2,8,CC);
          tft.fillRect(83,38,2,8,CC);tft.fillRect(87,38,2,8,CC);
          tft.fillRect(69,28,22,14,CC);
          tft.fillRect(65,34,4,4,CC);tft.fillRect(91,34,4,4,CC);
          int eH=min(2+gFrm,4);
          tft.fillRect(73,32,2,eH,0);tft.fillRect(85,32,2,eH,0);
          int bY=40+gFrm*3;
          if(bY<48){uint16_t bC=tft.color565(139,69,19);tft.fillRect(74,bY,12,5,bC);tft.fillRect(76,bY+1,8,3,0xFFFF);}
        }else{
          int yU=-min(gFrm,2);
          tft.fillRect(71,46,18,2,0x4208);
          tft.fillRect(71,38,2,8,CC);tft.fillRect(75,38,2,8,CC);
          tft.fillRect(83,38,2,8,CC);tft.fillRect(87,38,2,8,CC);
          tft.fillRect(69,28+yU,22,14,CC);
          tft.fillRect(65,34+yU,4,4,CC);tft.fillRect(91,34+yU,4,4,CC);
          int eH=1+gFrm;if(eH>4)eH=4;
          tft.fillRect(73,32+yU,2,eH,0);tft.fillRect(85,32+yU,2,eH,0);
        }
        tft.setTextSize(1);tft.setTextColor(0xFFE0);tft.setCursor(5,70);tft.print("thinking");
        if(gFrm>=4) gTrans=0;
      }else{
        gTrans=0;
        gFrm=(gFrm+1)%12;
        int xO=(gFrm<6)?-4:4;
        int ox=65+xO;
        tft.fillRect(ox+6,46,18,2,0x4208);
        tft.fillRect(ox+6,38,2,8,CC);tft.fillRect(ox+10,38,2,8,CC);
        tft.fillRect(ox+18,38,2,8,CC);tft.fillRect(ox+22,38,2,8,CC);
        tft.fillRect(ox+4,28,22,14,CC);
        int armOff=(gFrm<6)?-4:0;
        tft.fillRect(ox,34+armOff,4,4,CC);tft.fillRect(ox+26,34,4,4,CC);
        tft.fillRect(ox+8,32,2,3,0);tft.fillRect(ox+20,32,2,3,0);
        int bx=ox-8,by=2;
        tft.fillRoundRect(bx,by,18,16,3,0xFFFF);
        tft.fillRect(ox+4,22,2,2,0xFFFF);tft.fillRect(ox+8,25,1,1,0xFFFF);
        uint16_t dotC=tft.color565(37,104,145);
        if(gFrm%6>=1) tft.fillRect(bx+2,by+6,2,2,dotC);
        if(gFrm%6>=3) tft.fillRect(bx+6,by+6,2,2,dotC);
        if(gFrm%6>=5){
          tft.fillRect(bx+10,by+6,2,2,dotC);
          uint16_t sc=tft.color565(255,224,102);
          tft.fillRect(ox+12,0,2,2,sc);
          tft.fillRect(ox+12,2,2,2,sc);tft.fillRect(ox+12,-2,2,2,sc);
          tft.fillRect(ox+10,0,2,2,sc);tft.fillRect(ox+14,0,2,2,sc);
        }
        tft.setTextSize(1);tft.setTextColor(0xFFE0);tft.setCursor(5,70);tft.print("thinking");
      }
    }
  }
  if (gState == 2) {
    static unsigned long t=0;
    if(millis()-t>200){
      t=millis(); gFrm=(gFrm+1)%24;
      tft.fillScreen(0x0000);
      int yO=(gFrm%4<2)?0:2;
      uint16_t sbg=tft.color565(30,30,46);
      uint16_t sbr=tft.color565(45,45,61);
      tft.fillRect(66,4,27,22,sbg);
      tft.fillRect(66,4,27,2,sbr);
      tft.fillRect(67,5,2,1,0xF800); tft.fillRect(70,5,2,1,0xFFE0); tft.fillRect(73,5,2,1,0x07E0);
      uint16_t cg=tft.color565(76,175,80);
      uint16_t cb=tft.color565(64,196,255);
      uint16_t cy=tft.color565(255,193,7);
      int li=gFrm/4;
      tft.fillRect(67,8,10,1,cg); tft.fillRect(69,11,16,1,cg);
      tft.fillRect(69,14,(9+li)%16,1,cb); tft.fillRect(67,17,(4+li)%8,1,cy);
      uint16_t db=tft.color565(64,196,255);
      if(gFrm%6<3) tft.fillRect(71,1,2,2,db);
      if(gFrm%6>=3) tft.fillRect(83,0,2,2,db);
      tft.fillRect(71,46,18,2,0x4208);
      tft.fillRect(71,38,2,8,CC); tft.fillRect(75,38,2,8,CC);
      tft.fillRect(83,38,2,8,CC); tft.fillRect(87,38,2,8,CC);
      tft.fillRect(69,28+yO,22,14,CC);
      uint16_t kc=tft.color565(69,90,100);
      uint16_t kk=tft.color565(120,144,156);
      tft.fillRect(64,40,32,6,kc);
      tft.fillRect(66,41,28,1,kk);
      tft.fillRect(66,43,28,1,kk);
      int aL=(gFrm%2==0)?-2:0;
      int aR=(gFrm%2==1)?-2:0;
      tft.fillRect(65,34+yO+aL,4,4,CC); tft.fillRect(91,34+yO+aR,4,4,CC);
      tft.fillRect(73,32+yO,2,3,0); tft.fillRect(85,32+yO,2,3,0);
      tft.setTextSize(1); tft.setTextColor(0x07E0); tft.setCursor(5,70); tft.print("typing");
    }
  }
  if (gState == 3) {
    static unsigned long t=0;
    if(millis()-t>180){
      t=millis(); gFrm=gFrm+1;
      tft.fillScreen(0x0000);
      int phase=gFrm%8;
      int jump=0;
      if(phase>=1 && phase<=3) jump=-(phase*(4-phase))*2;
      uint16_t gold=tft.color565(255,215,0);
      uint16_t amb=tft.color565(255,193,7);
      uint16_t ly=tft.color565(255,249,157);
      if(gFrm%4<2){
        tft.fillRect(57,12,2,2,gold); tft.fillRect(57,10,2,2,gold); tft.fillRect(57,14,2,2,gold);
        tft.fillRect(55,12,2,2,gold); tft.fillRect(59,12,2,2,gold);
        tft.fillRect(101,8,2,2,amb); tft.fillRect(101,6,2,2,amb); tft.fillRect(101,10,2,2,amb);
        tft.fillRect(99,8,2,2,amb); tft.fillRect(103,8,2,2,amb);
      } else {
        tft.fillRect(105,36,2,2,ly); tft.fillRect(105,34,2,2,ly); tft.fillRect(105,38,2,2,ly);
        tft.fillRect(103,36,2,2,ly); tft.fillRect(107,36,2,2,ly);
        tft.fillRect(53,40,2,2,amb); tft.fillRect(53,38,2,2,amb); tft.fillRect(53,42,2,2,amb);
        tft.fillRect(51,40,2,2,amb); tft.fillRect(55,40,2,2,amb);
      }
      tft.fillRect(71,46,18,2,0x4208);
      tft.fillRect(69,28+jump,22,14,CC);
      tft.fillRect(71,38+jump,2,4,CC); tft.fillRect(75,38+jump,2,4,CC);
      tft.fillRect(83,38+jump,2,4,CC); tft.fillRect(87,38+jump,2,4,CC);
      tft.fillRect(65,34+jump,4,4,CC); tft.fillRect(91,34+jump,4,4,CC);
      bool hblink=(gFrm%8==4);
      if(!hblink){tft.fillRect(73,32+jump,2,4,0); tft.fillRect(85,32+jump,2,4,0);}
      tft.setTextSize(1); tft.setTextColor(0x07E0); tft.setCursor(5,70); tft.print("done!");
      if(!gMut && gFrm<16){int fq[]={523,659,784,1047,0,1319,1568,2093,0,1047,1568,0,2093,0,0,0}; if(fq[gFrm]>0) tone(4,fq[gFrm],150);}
    }
  }
  if (gState == 4) {
    static unsigned long t=0;
    if(millis()-t>400){
      t=millis(); gFrm=!gFrm;
      tft.fillScreen(gFrm ? 0xC520 : 0x0000);
      tft.fillRect(71,46,18,2,0x4208);
      tft.fillRect(71,38,2,8,CC); tft.fillRect(75,38,2,8,CC);
      tft.fillRect(83,38,2,8,CC); tft.fillRect(87,38,2,8,CC);
      tft.fillRect(69,28,22,14,CC);
      tft.fillRect(65,34,4,4,CC); tft.fillRect(91,34,4,4,CC);
      tft.fillRect(73,32,2,4,0); tft.fillRect(85,32,2,4,0);
      tft.fillRoundRect(80,2,18,16,3,0xFFE0);
      tft.fillRoundRect(82,4,14,12,2,0x0000);
      tft.setTextSize(2); tft.setTextColor(0xFFE0); tft.setCursor(85,5); tft.print("?");
      uint16_t tc=gFrm ? 0xF800 : 0xFFE0;
      tft.setTextSize(1); tft.setTextColor(tc); tft.setCursor(5,70); tft.print("Waiting");
      }
  }
  if (millis() >= gSolT) {
    digitalWrite(3, LOW);
  }
  delay(30);

}